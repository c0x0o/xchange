#ifndef _BASE_LOCKFREEQUEUE_H_
#define _BASE_LOCKFREEQUEUE_H_

#include <pthread.h>
#include <string.h>

#include <atomic>
#include <stdexcept>

#define round_u32(n) ({uint32_t i = 1; while (i < n)i<<=1;i;})
#define mod(a,b) (a&(b-1))

namespace xchange {

    template<typename T>
    class LockFreeQueue {
        public:
            LockFreeQueue(uint32_t max)
            {
                maxSize_ = round_u32(max);
                queue_ = new T[maxSize_];
                currentTail_.store(0, std::memory_order_relaxed);
                head_.store(0, std::memory_order_relaxed);
                tail_.store(0, std::memory_order_relaxed);
            }
            LockFreeQueue(const LockFreeQueue<T> & oldQueue) {
                maxSize_ = oldQueue.maxSize_;
                memcpy(&queue_, &oldQueue.queue_, maxSize_);
                currentTail_.store(oldQueue.currentTail_.load(std::memory_order_relaxed), std::memory_order_relaxed);
                head_.store(oldQueue.head_.load(std::memory_order_relaxed), std::memory_order_relaxed);
                tail_.store(oldQueue.tail_.load(std::memory_order_relaxed), std::memory_order_relaxed);
            }
            ~LockFreeQueue() {
                delete queue_;
            }

            uint32_t size() const {return currentTail_.load(std::memory_order_relaxed) - head_.load(std::memory_order_relaxed);}
            bool empty() const {return currentTail_.load(std::memory_order_relaxed) == head_.load(std::memory_order_relaxed);}

            T shift() {
                uint32_t readIndex;
                uint32_t maxReadIndex;

                do {
                    readIndex = head_.load(std::memory_order_relaxed);
                    maxReadIndex = currentTail_.load(std::memory_order_relaxed);

                    if (mod(readIndex, maxSize_) == mod(maxReadIndex, maxSize_)) {
                        throw std::out_of_range("out of range in LockFreeQueue");
                    }
                } while (!std::atomic_compare_exchange_weak_explicit(&head_, &readIndex, readIndex+1,
                            std::memory_order_release,
                            std::memory_order_relaxed));

                return queue_[mod(readIndex, maxSize_)];
            };

            void push(T val) {
                uint32_t readIndex;
                uint32_t writeIndex;

                do {
                    readIndex = head_.load(std::memory_order_relaxed);
                    writeIndex = tail_.load(std::memory_order_relaxed);

                    if (mod(writeIndex+1, maxSize_) == mod(readIndex, maxSize_)) {
                        throw std::overflow_error("overflow in LockFreeQueue");
                    }
                } while (!std::atomic_compare_exchange_weak_explicit(&tail_, &writeIndex, writeIndex+1,
                            std::memory_order_release,
                            std::memory_order_relaxed));

                queue_[mod(writeIndex, maxSize_)] = val;

                while (!std::atomic_compare_exchange_weak_explicit(&currentTail_, &writeIndex, writeIndex+1,
                            std::memory_order_release,
                            std::memory_order_relaxed)) {
                    pthread_yield();
                }
            };

        private:
            T *queue_;
            uint32_t maxSize_;
            std::atomic_uint32_t currentTail_, head_, tail_;
    };
}

#endif
