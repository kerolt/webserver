#include "LFUCache.h"

void KeyList::init(int fq) {
    freq = fq;
    // head_=last=new Node<Key>;
    head = last = NewElement<Node<Key>>();
    head->setNext(nullptr);
}

void KeyList::destory() {
    while (head) {
        key_node pre = head;
        head = head->getNext();
        // delete pre;
        DeleteElement(pre);
    }
}

int KeyList::getFreq() {
    return freq;
}

void KeyList::add(key_node& node) {
    if (head->getNext()) {
        head->getNext()->setPre(node);
        // printf("is not one\n");
    } else
        last = node;
    node->setNext(head->getNext());
    node->setPre(head);
    head->setNext(node);
    // printf("last key=%d\n",last->getValue().key);
}

void KeyList::del(key_node& node) {
    node->getPre()->setNext(node->getNext());
    if (node->getNext())
        node->getNext()->setPre(node->getPre());
    else
        last = node->getPre();
}

bool KeyList::isEmpty() {
    return head == last;
}

key_node KeyList::getLast() {
    return last;
}

void LFUCache::Init() {
    capacity_ = GetConf().GetCapacity();
    // head_=new Node<KeyList>;
    head_ = NewElement<Node<KeyList>>();
    head_->getValue().init(0);
    head_->setNext(NULL);
}

LFUCache::~LFUCache() {
    while (head_) {
        freq_node pre = head_;
        head_ = head_->getNext();
        pre->getValue().destory();
        // delete pre;
        DeleteElement(pre);
    }
}

void LFUCache::AddFreq(key_node& now_key, freq_node& now_freq) {
    freq_node nxt;
    if (!now_freq->getNext() || now_freq->getNext()->getValue().getFreq() != now_freq->getValue().getFreq() + 1) {
        // 插入freqnode
        // printf("new freqnode!\n");
        // nxt=new Node<KeyList>;
        nxt = NewElement<Node<KeyList>>();
        nxt->getValue().init(now_freq->getValue().getFreq() + 1);
        if (now_freq->getNext())
            now_freq->getNext()->setPre(nxt);
        nxt->setNext(now_freq->getNext());
        now_freq->setNext(nxt);
        nxt->setPre(now_freq);
    } else
        nxt = now_freq->getNext();
    freq_map_[now_key->getValue().key] = nxt;
    // 移动key_node
    if (now_freq != head_) {
        now_freq->getValue().del(now_key);
    }
    nxt->getValue().add(now_key);
    if (now_freq != head_ && now_freq->getValue().isEmpty()) {
        Del(now_freq);
    }
}

bool LFUCache::Get(string& key, string& value) {
    if (!capacity_) {
        return false;
    }
    MutexLockGuard lock(mutex_);
    if (freq_map_.find(key) != freq_map_.end()) {
        // 命中
        key_node nowk = kv_map_[key];
        freq_node nowf = freq_map_[key];
        value += nowk->getValue().value;
        AddFreq(nowk, nowf);
        return true;
    }
    return false;
}

void LFUCache::Set(string& key, string& value) {
    if (!capacity_) {
        return;
    }
    // printf("kmapsize=%d capacity_=%d\n",kv_map_.size_(),capacity_);
    MutexLockGuard lock(mutex_);
    if (kv_map_.size() == capacity_) {
        freq_node headnxt = head_->getNext();
        key_node last = headnxt->getValue().getLast();
        headnxt->getValue().del(last);
        // printf("key=%d\n",last->getValue().key);
        kv_map_.erase(last->getValue().key);
        freq_map_.erase(last->getValue().key);
        // delete last;
        DeleteElement(last);
        if (headnxt->getValue().isEmpty())
            Del(headnxt);
    }
    // key_node now_key=new Node<Key>;
    key_node now_key = NewElement<Node<Key>>();
    now_key->getValue().key = key;
    now_key->getValue().value = value;
    AddFreq(now_key, head_);
    kv_map_[key] = now_key;
}

void LFUCache::Del(freq_node& node) {
    node->getPre()->setNext(node->getNext());
    if (node->getNext())
        node->getNext()->setPre(node->getPre());
    node->getValue().destory();
    // delete node;
    DeleteElement(node);
}

LFUCache& GetCache() {
    static LFUCache cache;
    return cache;
}
