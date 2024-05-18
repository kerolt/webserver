#pragma once

#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>

#include <utility>
// #include <functional>

#include "../mutex/MutexLock.h"

#define BLOCK_SIZE 4096

struct Slot {
    Slot* next;
};

class MemoryPool {
public:
    MemoryPool() = default;
    ~MemoryPool();
    void Init(int size);
    Slot* Allocate();
    void Deallocate(Slot* p);

private:
    int slot_size_{};

    Slot* cur_block_{};
    Slot* cur_slot_{};
    Slot* last_slot_{};
    Slot* free_slot_{};

    MutexLock m_freeSlot;
    MutexLock m_other;

    size_t PaddingSize(const char* p, size_t align);
    Slot* AllocateBlock();
    Slot* NofreeSolve();
};

void InitMemoryPool();

void* UseMemory(size_t size);

void FreeMemory(size_t size, void* p);

MemoryPool& GetMemoryPool(int id);

template <class T, class... Args>
T* NewElement(Args&&... args) {
    T* p;
    if ((p = reinterpret_cast<T*>(UseMemory(sizeof(T))))) {
        new (p) T(std::forward<Args>(args)...);
    }
    return p;
}

template <class T>
void DeleteElement(T* p) {
    if (p) {
        p->~T();
    }
    FreeMemory(sizeof(T), reinterpret_cast<void*>(p));
}
