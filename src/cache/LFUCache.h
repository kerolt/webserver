#pragma once

#include <string>
#include <unordered_map>

#include "../memory_pool/MemoryPool.h"
#include "../conf/Conf.h"
#include "../mutex/MutexLock.h"

using std::string;

template <typename T>
class Node {
private:
    T value;
    Node* pre;
    Node* next;

public:
    void setPre(Node* p) {
        pre = p;
    }
    void setNext(Node* n) {
        next = n;
    }
    Node* getPre() {
        return pre;
    }
    Node* getNext() {
        return next;
    }
    T& getValue() {
        return value;
    }
};

struct Key {
    string key, value;
};

typedef Node<Key>* key_node;

class KeyList {
private:
    int freq;
    key_node head;
    key_node last;

public:
    void init(int fq);
    void destory();
    int getFreq();
    void add(key_node& node);
    void del(key_node& node);
    bool isEmpty();
    key_node getLast();
};

typedef Node<KeyList>* freq_node;

// LFU缓存
class LFUCache {
private:
    freq_node head_;
    int capacity_;
    MutexLock mutex_;

    std::unordered_map<string, key_node> kv_map_;    // key到keynode的映射
    std::unordered_map<string, freq_node> freq_map_; // key到freqnode的映射

    void AddFreq(key_node& now_key, freq_node& now_freq);
    void Del(freq_node& node);

public:
    void Init();
    ~LFUCache();
    bool Get(string& key, string& value); // 通过key返回value并进行LFU操作
    void Set(string& key, string& value); // 更新LFU缓存
};

LFUCache& GetCache();
