#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>

#define OBJECT_COUNT 4
#define OBJECT_MASK (OBJECT_COUNT - 1)
#define COUNT_MASK (~OBJECT_MASK)
#define COUNT_INC OBJECT_COUNT
#define PERSISTENT 1
#define TEMPORAL 2

struct Object {
    std::atomic<uintptr_t> rc; // inner counter
    std::atomic<Object *> * back_ptr;
    void (*dtor)(void * obj);
};

struct Store {
    std::atomic<uintptr_t> state; // outer counter + index to objects
    std::array<std::atomic<Object *>, OBJECT_COUNT> objects;
    std::mutex mu;
};

void storeCreate(Store * store, Object * obj, void (*dtor)(void *)) {
    store->state.store(0);
    store->objects[0].store(obj);
    for (auto i = 1; i != OBJECT_COUNT; ++i)
        store->objects[i].store(nullptr);
    obj->rc.store(PERSISTENT);
    obj->back_ptr = &store->objects[0];
    obj->dtor = dtor;
}

static void storeReleaseObject(Object * obj) {
    obj->back_ptr->store(nullptr);
    obj->dtor(obj);
}

void storeDestroy(Store * store) {
    auto idx = store->state.load() & OBJECT_MASK;
    auto * obj = store->objects[idx].load();
    obj->rc.fetch_sub((store->state.load() & COUNT_MASK) / OBJECT_COUNT * TEMPORAL + PERSISTENT);
    storeReleaseObject(obj);
}

Object * storeReadRequire(Store * store) {
    auto prev = store->state.fetch_add(COUNT_INC);
    auto idx = prev & OBJECT_MASK;
    return store->objects[idx].load();
}

void storeReadRelease(Object * obj) {
    auto prev = obj->rc.fetch_add(TEMPORAL) + TEMPORAL;
    if (prev == 0) {
        storeReleaseObject(obj);
    }
}

Object * storeWriteLock(Store * store) {
    store->mu.lock();
    auto idx = store->state.load() & OBJECT_MASK;
    return store->objects[idx].load();
}

void storeWriteUnlock(Store * store, Object * obj, void (*dtor)(void *)) {
    uintptr_t idx = 0;
    for (;;) {
        for (idx = 0; idx != OBJECT_COUNT; ++idx) {
            if (store->objects[idx] == nullptr) {
                break;
            }
        }
        if (idx != OBJECT_COUNT) {
            break;
        }
        std::this_thread::yield();
    }
    store->objects[idx].store(obj);
    obj->rc = PERSISTENT;
    obj->back_ptr = &store->objects[idx];
    obj->dtor = dtor;
    auto prev = store->state.exchange(idx);
    auto old_cnt = prev & COUNT_MASK;
    auto old_idx = prev & OBJECT_MASK;
    auto * old_obj = store->objects[old_idx].load();
    auto cnt_dif = static_cast<uintptr_t>(-static_cast<intptr_t>(old_cnt / OBJECT_COUNT * TEMPORAL + PERSISTENT));
    auto cnt_res = old_obj->rc.fetch_add(cnt_dif) + cnt_dif;
    store->mu.unlock();
    if (cnt_res == 0) {
        storeReleaseObject(old_obj);
    }
}
