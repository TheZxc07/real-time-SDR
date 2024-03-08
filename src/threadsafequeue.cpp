//#include "threadsafequeue.h"
/*
template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() {}


template <typename T>
void ThreadSafeQueue<T>::push(const T& value) {
    std::lock_guard<std::mutex> lock(m);
    data_queue.push(value);
    cv.notify_one();
}

template <typename T>
bool ThreadSafeQueue<T>::try_pop(T& value) {
    std::lock_guard<std::mutex> lock(m);
    if (data_queue.empty())
        return false;
    value = data_queue.front();
    data_queue.pop();
    return true;
}

template <typename T>
void ThreadSafeQueue<T>::wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this] { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(m);
    return data_queue.empty();
}

*/
