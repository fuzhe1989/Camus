#include <atomic>
#include <cassert>
#include <cstdint>
#include <tuple>

// This file is based on the following paper:
// https://dl.acm.org/doi/pdf/10.1145/2851141.2851168
//
// Note that the author already opensourced his code at https://github.com/chaoran/fast-wait-free-queue

template <typename T, typename U>
T cas(std::atomic<T> & c, U expected, T to) {
    T e = expected;
    c.compare_exchange_strong(e, to);
    return e;
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
    // difference between id and state.id:
    // - id only changed when start a new request
    // - state.id changed during the processing of a request
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
            if (cas(s->next, nullptr, tmp) != nullptr) {
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
        if (v >= cid || cas(*e, v, cid) == v) {
            break;
        }
    }
}

bool tryToClaimReq(std::atomic<State> * s, int64_t id, int64_t cellId, State * newState) {
    *newState = {true, id};
    State to = {false, cellId};
    return cas(*s, *newState, to) == *newState;
}

void enqCommit(Queue * q, Cell * c, void * v, int64_t cid) {
    // keep reverse order of `helpEnqueue`
    // Note that `enqCommit` is only called on slow path, while on fast path q->tail is directly FAAed.
    advanceEndForLinearizability(&q->tail, cid + 1);
    c->val.store(v);
}

bool enqueueFast(Queue * q, Handle * h, void * v, int64_t * cid) {
    auto i = q->tail.fetch_add(1);
    auto * c = findCell(&h->tail, i);
    if (cas(c->val, nullptr, v) == nullptr)
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
        if (cas(c->enq, nullptr, r) == nullptr) {
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
    if (auto * cellValue = cas(c->val, nullptr, gNeverValue); cellValue != nullptr) {
        // c->val could be v or 'never', there won't have way from v to 'nevee' so directly compare is ok.
        if (cellValue != gNeverValue) {
            return cellValue;
        }
    }
    // Now c->val is 'never', try to help an enqueuer for occupying this cell
    // No enqueuer here, try to help peer
    auto * enq = c->enq.load();
    if (enq == nullptr) {
        auto * p = h->enqPeer;
        auto * r = &p->enqReq;
        auto s = r->state.load();
        auto hs = h->enqReq.state.load();
        // hs.id == s.id means I saw it last round. Try to help this peer.
        if (hs.id != s.id) {
            // try next peer
            h->enqPeer = p = p->next;
            r = &p->enqReq;
            s = r->state.load();
        }
        // s.pending: s really needs help
        // s.id <= i: this help won't violate linearizability
        // cas: help succeeded
        if (s.pending && s.id <= i && (enq = cas(c->enq, nullptr, r)) != nullptr) {
            // Fail to help current peer, there must someone (enqueuer or dequeuer) is doing the same thing.
            // Break and try to help this enq.
            // Record s.id for next `helpEnqueue`
            h->enqReq.state.store({false, s.id});
        } else {
            // This peer needn't help or the cas already succeeded.
            // In both scenes we could move to next peer.
            h->enqPeer = p->next;
        }
        if (enq == nullptr) {
            // Try to mask this cell as 'never'
            enq = cas(c->enq, nullptr, gNeverEnq);
        }
    }
    // If enq is 'never' then no enqueuer will enter here.
    if (enq == gNeverEnq) {
        // q->tail <= i means the dequeuer is too ahead of enqueuers and don't worth to retry.
        // q->tail > i means the dequeuer could continue to search for a candidate.
        return q->tail.load() <= i ? nullptr : gNeverValue;
    }
    auto [v, s] = enq->loadBoth();
    // s.id > i means the enqueuer already skipped this cell
    if (s.id > i) {
        // must fetch val then tail, keep reserve order of `enqCommit`
        // same as the previous if
        if (c->val.load() == gNeverValue && q->tail.load() <= i)
            return nullptr;
        else
            return gNeverValue;
    } else if (tryToClaimReq(&enq->state, s.id, i, &s)) {
        // claim succeeds, continue commit.
        enqCommit(q, c, v, i);
        return v;
    } else if (s == State{false, i} && c->val.load() == gNeverValue) {
        // claim failed and another guy also claimed i and they hasn't finished commit.
        // Try to help they.
        enqCommit(q, c, v, i);
        return v;
    } else {
        // claim failed and another guy claimed a different value, return the new value.
        return c->val.load();
    }
}

// note that multiple dequeuers may be working on the same Handle.
void helpDequeue(Queue * q, Handle * h, Handle * helpee) {
    auto * ha = helpee->head; // avoid directly modify peer's head
    auto * r = &helpee->deqReq;
    auto [id, s] = r->loadBoth();
    // !s.pending: s needn't help
    // s.id < id: s falls behind and we can't help they.
    if (!s.pending || s.id < id)
        return;
    // prior is a flag for acknowledging changes to s.id
    auto prior = id;
    // i: last searched id (may be or may not be cand)
    auto i = id;
    int64_t cand = 0;
    for (;;) {
        // s.id != prior: there are two change points, one at `dequeueSlow` which denotes a new request,
        //   one at below denotes a candidate is found. In neither case should we continue the search.
        for (auto * hc = ha; !cand && s.id == prior;) {
            auto * c = findCell(&hc, ++i);
            auto * v = helpEnqueue(q, h, c, i);
            // v == nullptr: no enqueuers reached i, we could wait here. TODO
            // v has value and seems no other dequeuers claimed v, try to claim it.
            if (v == nullptr || (v != gNeverValue && c->deq.load() == nullptr))
                cand = i;
            else
                // check if s is changed, see above
                s = r->state.load();
        }
        if (cand) {
            // Claim r's target is cand now. Also interrupt other helpers.
            cas(r->state, State{true, prior}, {true, cand});
            s = r->state.load();
        }
        // !s.pending: helpee already finished.
        // r->id != id: helpee started a new request.
        if (!s.pending || r->id.load() != id)
            return;
        auto * c = findCell(&ha, s.id);
        // c->val == gNeverValue: terminal status, c is over.
        // cas succeed: claimed succeed.
        // c->deq == r: another helper did what we intend to do.
        if (c->val == gNeverValue || cas(c->deq, nullptr, r) == nullptr || c->deq.load() == r) {
            // finish this request
            cas(r->state, s, {false, s.id});
            return;
        }
        // cand == 0 or i
        // i > prior
        // s.id could be cand or another cand from another helpee.
        // persistent s.id to prior for check at L304.
        // the if is to ensure the monotonicity of our search.
        prior = s.id;
        if (s.id >= i) {
            cand = 0;
            i = s.id;
        }
    }
}

std::tuple<int64_t, void *> dequeueFast(Queue * q, Handle * h) {
    auto i = q->head.fetch_add(1);
    auto * c = findCell(&h->head, i);
    auto * v = helpEnqueue(q, h, c, i);
    // nullptr means the current dequeuer is ahead of all enqueuers.
    // Return nullptr means this call failed but this cell is revisitable.
    if (v == nullptr)
        return {i, nullptr};
    // v == gNeverValue: this cell is marked as 'never', skip it.
    // cas fail: a dequeuer already working on this cell, skip it.
    if (v != gNeverValue && cas(c->deq, nullptr, gNeverDeq) == nullptr)
        return {i, v};
    return {i, gNeverValue};
}

void * dequeueSlow(Queue * q, Handle * h, int64_t cid) {
    auto * r = &h->deqReq;
    // claim self as help needed
    r->storeBoth(cid, {true, cid});
    // help self
    helpDequeue(q, h, h);
    // r contains the result of `helpDequeue`
    auto i = r->state.load().id;
    auto * c = findCell(&h->head, i);
    auto * v = c->val.load();
    // cells before i+1 are all scaned. Note that in the fast path q->head is directly FAAed.
    advanceEndForLinearizability(&q->head, i + 1);
    // if v is still invalid, that means the dequeuer is ahead of all enqueuers. Return empty
    return v == gNeverValue ? nullptr : v;
}

void * dequeue(Queue * q, Handle * h) {
    void * v = nullptr;
    int64_t cellId = 0;
    for (auto p = PATIENCE; p >= 0; --p) {
        std::tie(cellId, v) = dequeueFast(q, h);
        // gNeverValue means we need to find for next cell
        if (v != gNeverValue)
            break;
    }
    if (v == gNeverValue)
        v = dequeueSlow(q, h, cellId); // lost PATIENCE
    // got value, do not return too early, try to help peer
    if (v != nullptr) {
        helpDequeue(q, h, h->deqPeer);
        h->deqPeer = h->deqPeer->next;
    }
    return v;
}
