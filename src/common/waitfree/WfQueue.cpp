#include <atomic>
#include <cstdint>

char gSealedHolder;
void * gSealedValue = &gSealedHolder;

char gInitHolder;
void * gInitValue = &gInitHolder;

struct State {
    bool pending : 1;
    int64_t id : 63;
};
static_assert(sizeof(State) == sizeof(int64_t), "");

struct EnqueueReq {
    void * val;
    State state;
};

struct DequeueReq {
    int64_t id;
    State state;
};

EnqueueReq gInitEnqHolder;
EnqueueReq * gInitEnq = &gInitEnqHolder;

DequeueReq gInitDeqHolder;
DequeueReq * gInitDeq = &gInitDeqHolder;

struct Cell {
    void * val = gInitValue;
    EnqueueReq * enq = gInitEnq;
    DequeueReq * deq = gInitDeq;
};

constexpr int64_t N = 64;

struct Segment {
    int64_t id = 0;
    std::atomic<Segment *> next{nullptr};
    Cell cells[N];
};

struct Handle {
    Segment * tail;
    Segment * head;
    Handle * next;
    struct {
        EnqueueReq req;
        Handle * peer;
    } enq;
    struct {
        DequeueReq req;
        Handle * peer;
    } deq;
};

Segment * newSegment(int64_t id) {
    auto * s = new Segment;
    s->id = id;
    return s;
}

Cell * findCell(Segment ** sp, int64_t cellId) {
    auto * s = *sp;
    for (auto i = s->id, sz = cellId / N; i < sz; ++i) {
        auto * next = s->next.load();
        if (!next) {
            auto * tmp = newSegment(i + 1);
            Segment * expected = nullptr;
            if (!s->next.compare_exchange_strong(expected, tmp)) {
                delete tmp;
            }
            next = s->next;
        }
        s = next;
    }
    *sp = s;
    return &s->cells[cellId % N];
}
