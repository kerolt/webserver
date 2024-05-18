#include "MemoryPool.h"

void MemoryPool::Init(int size) {
    slot_size_ = size;
    cur_block_ = cur_slot_ = last_slot_ = free_slot_ = nullptr;
}

MemoryPool::~MemoryPool() {
    Slot* curr = cur_block_;
    while (curr) {
        Slot* prev = curr->next;
        free(reinterpret_cast<void*>(curr));
        // operator delete(reinterpret_cast<void *>(curr));
        curr = prev;
    }
}

inline size_t MemoryPool::PaddingSize(const char* p, size_t align) {
    uintptr_t result = reinterpret_cast<uintptr_t>(p);
    // printf("align=%lu\n",align);
    return ((align - result) % align);
}

Slot* MemoryPool::AllocateBlock() {
    char* newBlock = nullptr;
    while (!(newBlock = reinterpret_cast<char*>(malloc(BLOCK_SIZE))))
        ;
    char* body = newBlock + sizeof(Slot);
    // printf("slot_size_=%d\n",slot_size_);
    size_t bodyPadding = PaddingSize(body, static_cast<size_t>(slot_size_));
    // data_pointer newBlock=reinterpret_cast<data_pointer>(operator new(BLOCK_SIZE));
    Slot* useSlot;
    {
        MutexLockGuard lock(m_other);
        reinterpret_cast<Slot*>(newBlock)->next = cur_block_;
        cur_block_ = reinterpret_cast<Slot*>(newBlock);
        // char* body=newBlock+sizeof(Slot);
        // size_t bodyPadding=PaddingSize(body,slot_size_);
        cur_slot_ = reinterpret_cast<Slot*>(body + bodyPadding);
        last_slot_ = reinterpret_cast<Slot*>(newBlock + BLOCK_SIZE - slot_size_ + 1);
        useSlot = cur_slot_;
        cur_slot_ += (slot_size_ >> 3);
    }
    return useSlot;
}

Slot* MemoryPool::NofreeSolve() {
    if (cur_slot_ >= last_slot_)
        return AllocateBlock();
    Slot* useSlot;
    {
        MutexLockGuard lock(m_other);
        useSlot = cur_slot_;
        cur_slot_ += (slot_size_ >> 3);
    }
    return useSlot;
}

Slot* MemoryPool::Allocate() {
    if (free_slot_) {
        {
            MutexLockGuard lock(m_freeSlot);
            if (free_slot_) {
                Slot* result = free_slot_;
                free_slot_ = free_slot_->next;
                return result;
            }
        }
    }
    return NofreeSolve();
}

inline void MemoryPool::Deallocate(Slot* p) {
    if (p) {
        MutexLockGuard lock(m_freeSlot);
        p->next = free_slot_;
        free_slot_ = p;
    }
}

/*void *operator new(size_t size_){
    printf("size_=%d\n",size_);
    long long *p;
    if(size_>128){

        //malloc未处理
        return malloc(size_);
    }
    return reinterpret_cast<void *>(memorypool[((size_+7)>>3)-1].Allocate());
}*/

void* UseMemory(size_t size) {
    if (!size) {
        return nullptr;
    }
    if (size > 512) {
        return malloc(size);
    }
    return reinterpret_cast<void*>(GetMemoryPool(((size + 7) >> 3) - 1).Allocate());
}

void FreeMemory(size_t size, void* p) {
    if (!p) {
        return;
    }
    if (size > 512) {
        free(p);
        return;
    }
    GetMemoryPool(((size + 7) >> 3) - 1).Deallocate(reinterpret_cast<Slot*>(p));
}

/*void operator delete(void *p,size_t size_){
    printf("exec delete!\n");
    if(size_>128)
        free(p);
    else
        memorypool[((size_+7)>>3)-1].Deallocate(reinterpret_cast<Slot *>(p));
}*/

void InitMemoryPool() {
    for (int i = 0; i < 64; ++i) {
        GetMemoryPool(i).Init((i + 1) << 3);
    }
}

MemoryPool& GetMemoryPool(int id) {
    static MemoryPool memorypool[64];
    return memorypool[id];
}
