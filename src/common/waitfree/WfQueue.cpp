#include <atomic>
#include <cassert>
#include <cstdint>
#include <tuple>

template <typename T, typename U>
bool cas(std::atomic<T> & c, U expected, T to) {
    T e = expected;
    return c.compare_exchange_strong(e, to);
}

template <typename T, typename U>
bool cas(std::atomic<T> & c, U expected, T to, T * newValue) {
    *newValue = expected;
    return c.compare_exchange_strong(*newValue, to);
}

struct State {
    bool pending : 1 = false;
    int64_t id : 63 = 0;

    bool operator==(const State & other) const { return pending == other.pending && id == other.id; }
    bool operator!=(const State & other) const { return !(*this == other); }
};
static_assert(sizeof(State) == sizeof(int64_t), "");

struct EnqueueReq {
    // both shared between enqueueSlow and helpEnqueue
    std::atomic<void *> val = nullptr;
    std::atomic<State> state;

    // used in enqueueSlow
    void storeBoth(void * v, State s) {
        val.store(v);
        state.store(s);
    }

    // used in helpEnqueue
    std::tuple<void *, State> loadBoth() const {
        // load in reverse order
        auto s = state.load();
        auto * v = val.load();
        return std::make_tuple(v, s);
    }
};

struct DequeueReq {
    // both shared between dequeueSlow and helpDequeue
    std::atomic<int64_t> id = 0;
    std::atomic<State> state;

    // used in dequeueSlow
    void storeBoth(int64_t i, State s) {
        id.store(i);
        state.store(s);
    }

    // used in helpEnqueue
    std::tuple<int64_t, State> loadBoth() const {
        // load in reverse order
        auto s = state.load();
        auto i = id.load();
        return std::make_tuple(i, s);
    }
};

char gNeverHolder;
void * gNeverValue = &gNeverHolder;
EnqueueReq * gNeverEnq = reinterpret_cast<EnqueueReq *>(gNeverValue);
DequeueReq * gNeverDeq = reinterpret_cast<DequeueReq *>(gNeverValue);

struct Cell {
    std::atomic<void *> val = nullptr;
    std::atomic<EnqueueReq *> enq = nullptr;
    std::atomic<DequeueReq *> deq = nullptr;
};

constexpr int64_t N = 64;
constexpr int64_t PATIENCE = 10;

struct Segment {
    const int64_t id = 0;
    std::atomic<Segment *> next{nullptr};
    Cell cells[N];

    explicit Segment(int64_t id_)
        : id(id_) {}
};

struct Queue {
    Segment * q = nullptr;
    std::atomic<int64_t> tail = 0;
    std::atomic<int64_t> head = 0;
};

struct Handle {
    Segment * tail;
    Segment * head;
    Handle * next;

    // used in helpEnqueue
    EnqueueReq enqReq;
    Handle * enqPeer;

    DequeueReq deqReq;
    Handle * deqPeer;
};

Segment * newSegment(int64_t id) {
    return new Segment(id);
}

Cell * findCell(Segment ** sp, int64_t cellId) {
    auto * s = *sp;
    for (auto i = s->id, sz = cellId / N; i < sz; ++i) {
        auto * next = s->next.load();
        if (!next) {
            auto * tmp = newSegment(i + 1);
            if (!cas(s->next, nullptr, tmp)) {
                delete tmp;
            }
            next = s->next;
        }
        s = next;
    }
    *sp = s;
    return &s->cells[cellId % N];
}

void advanceEndForLinearizability(std::atomic<int64_t> * e, int64_t cid) {
    for (;;) {
        auto v = e->load();
        if (v >= cid || cas(*e, v, cid)) {
            break;
        }
    }
}

bool tryToClaimReq(std::atomic<State> * s, int64_t id, int64_t cellId, State * newState) {
    *newState = {true, id};
    State to = {false, cellId};
    return cas(*s, *newState, to);
}

void enqCommit(Queue * q, Cell * c, void * v, int64_t cid) {
    // keep reverse order from helpEnqueue
    advanceEndForLinearizability(&q->tail, cid + 1);
    c->val.store(v);
}

bool enqueueFast(Queue * q, Handle * h, void * v, int64_t * cid) {
    auto i = q->tail.fetch_add(1);
    auto * c = findCell(&h->tail, i);
    if (cas(c->val, nullptr, v))
        return true;
    *cid = i;
    return false;
}

void enqueueSlow(Queue * q, Handle * h, void * v, int64_t cellId) {
    auto * r = &h->enqReq;
    r->storeBoth(v, {true, cellId}); // may be seen by peer (helpEnqueue)

    // avoid directly modifying h->tail: gc may be also modifying it.
    auto * tmpTail = h->tail;
    State s;
    for (;;) {
        auto i = q->tail.fetch_add(1);
        auto * c = findCell(&tmpTail, i);
        // Try to prevent other dequeuers from occupying this cell.
        // Note that other enqueuers won't touch this cell since all enqueuers are FAA q->tail.
        if (cas(c->enq, nullptr, r)) {
            // Must cas c->enq then load c->val since dequeuer follows the same order.
            auto * v = c->val.load();
            // Now helpEnqueue won't change c->enq anymore, but some helper may already mark it as 'never'.
            // In this case enqueuer couldn't write this cell or the value won't be dequeued.
            if (v == nullptr) {
                // Now we arrive a safe point: (e, r) always ends with a successful enqueue.
                // If `tryToClaimReq` failed, there must be a helper have done this.
                tryToClaimReq(&r->state, cellId, i, &s);
                break;
            } else {
                // one dequeuer already marked this cell as 'never', try again
                assert(v == gNeverValue);
            }
        } else {
            // one dequeuer already chose this cell for helpEnqueue, skip it and try again
        }
        // check if someone already helped me.
        s = r->state.load();
        if (!s.pending) {
            break;
        }
    }
    // s is guaranteed loaded just before loop break
    auto id = s.id;
    auto * c = findCell(&h->tail, id);
    // publish v to c
    enqCommit(q, c, v, id);
}

void enqueue(Queue * q, Handle * h, void * v) {
    int64_t cellId = 0;
    for (auto p = PATIENCE; p >= 0; --p) {
        if (enqueueFast(q, h, v, &cellId))
            return;
    }
    // always succeed
    enqueueSlow(q, h, v, cellId);
}

void * helpEnqueue(Queue * q, Handle * h, Cell * c, int64_t i) {
    // try mark an empty cell as 'never'
    void * cellValue = nullptr;
    if (!cas(c->val, nullptr, gNeverValue, &cellValue)) {
        // c->val could be v or 'never', there won't have way from v to 'nevee' so directly compare is ok.
        if (cellValue != gNeverValue) {
            return cellValue;
        }
    }
    // Now c->val is 'never', try to help an enqueuer for occupying this cell
    // No enqueuer here, try to help peer
    if (c->enq.load() == nullptr) {
        State s;
        EnqueueReq * r = nullptr;
        Handle * p = nullptr;
        for (;;) {
            p = h->enqPeer;
            r = &p->enqReq;
            s = r->state.load();
            auto hs = h->enqReq.state.load();
            if (hs.id == 0 || hs.id == s.id)
                break;
            h->enqReq.state.store({hs.pending, 0});
            h->enqPeer = p->next;
        }
        if (s.pending && s.id <= i && !cas(c->enq, nullptr, r)) {
            h->enqReq.state.store({false, s.id});
        } else {
            h->enqPeer = p->next;
        }
        if (c->enq.load() == nullptr)
            cas(c->enq, nullptr, gNeverEnq);
    }
    if (c->enq.load() == gNeverEnq) {
        return q->tail <= i ? nullptr : gNeverValue;
    }
    auto * r = c->enq.load();
    auto [v, s] = r->loadBoth();
    if (s.id > i) {
        if (c->val == gNeverValue && q->tail <= i)
            return nullptr;
    } else if (tryToClaimReq(&r->state, s.id, i, &s) || (s == State{false, i} && c->val == gNeverValue)) {
        enqCommit(q, c, v, i);
    }
    return c->val;
}

void helpDequeue(Queue * q, Handle * h, Handle * helpee) {
    auto * r = &helpee->deqReq;
    auto [id, s] = r->loadBoth();
    if (!s.pending || s.id < id)
        return;
    auto * ha = helpee->head;
    s = r->state.load();
    auto prior = id;
    auto i = id;
    int64_t cand = 0;
    for (;;) {
        for (auto * hc = ha; !cand && s.id == prior;) {
            auto * c = findCell(&hc, ++i);
            auto * v = helpEnqueue(q, h, c, i);
            if (v == nullptr || (v != gNeverValue && c->deq.load() == nullptr))
                cand = i;
            else
                s = r->state.load();
        }
        if (cand) {
            cas(r->state, State{true, prior}, {true, cand});
            s = r->state.load();
        }
        if (!s.pending || r->id.load() != id)
            return;
        auto * c = findCell(&ha, s.id);
        if (c->val == gNeverValue || cas(c->deq, nullptr, r) || c->deq.load() == r) {
            cas(r->state, s, {false, s.id});
            return;
        }
        prior = s.id;
        if (s.id >= i) {
            cand = 0;
            i = s.id;
        }
    }
}

void * dequeueFast(Queue * q, Handle * h, int64_t * id) {
    auto i = q->head.fetch_add(1);
    auto * c = findCell(&h->head, i);
    auto * v = helpEnqueue(q, h, c, i);
    if (v == nullptr)
        return nullptr;
    if (v != gNeverValue && cas(c->deq, nullptr, gNeverDeq))
        return v;
    *id = i;
    return gNeverValue;
}

void * dequeueSlow(Queue * q, Handle * h, int64_t cid) {
    auto * r = &h->deqReq;
    r->storeBoth(cid, {true, cid});

    helpDequeue(q, h, h);
    auto i = r->state.load().id;
    auto * c = findCell(&h->head, i);
    auto * v = c->val.load();
    advanceEndForLinearizability(&q->head, i + 1);
    return v == gNeverValue ? nullptr : v;
}

void * dequeue(Queue * q, Handle * h) {
    void * v = nullptr;
    int64_t cellId = 0;
    for (auto p = PATIENCE; p >= 0; --p) {
        v = dequeueFast(q, h, &cellId);
        if (v != gNeverValue)
            break;
    }
    if (v == gNeverValue)
        v = dequeueSlow(q, h, cellId);
    if (v != nullptr) {
        helpDequeue(q, h, h->deqPeer);
        h->deqPeer = h->deqPeer->next;
    }
    return v;
}
