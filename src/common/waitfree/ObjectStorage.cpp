#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>

#define OBJECT_COUNT 4
#define OBJECT_MASK (OBJECT_COUNT - 1)
#define COUNT_MASK (~OBJECT_MASK)
#define COUNT_INC OBJECT_COUNT
#define PERSISTENT 1
#define TEMPORAL 2

struct Object {
    uintptr_t rc; // inner counter
    std::atomic<Object *> * back_ptr;
    void (*dtor)(void * obj);
};

struct Store {
    std::atomic<uintptr_t> state; // outer counter + index to objects
    std::array<std::atomic<Object *>, OBJECT_COUNT> objects;
    std::mutex mu;
};

void storeCreate(Store * store, Object * obj, void (*dtor)(void *)) {
    store->state.store(0, std::memory_order_relaxed);
    store->objects[0].store(obj, std::memory_order_relaxed);
    for (auto i = 1; i != OBJECT_COUNT; ++i)
        store->objects[i].store(nullptr, std::memory_order_relaxed);
    obj->rc = PERSISTENT;
    obj->back_ptr = &store->objects[0];
    obj->dtor = dtor;
}

static void storeReleaseObject(Object * obj) {
    obj->back_ptr->store(nullptr, std::memory_order_relaxed);
    obj->dtor(obj);
}

void storeDestroy(Store * store) {
    auto idx = store->state.load(std::memory_order_relaxed) & OBJECT_MASK;
    auto * obj = store->objects[idx].load(std::memory_order_relaxed);
    // TODO
}
