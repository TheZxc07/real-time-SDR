#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    
    mutable std::mutex m;
    std::condition_variable cv;

public:
std::queue<T> data_queue;
    ThreadSafeQueue() {}
    void push(const T& value) {
		std::lock_guard<std::mutex> lock(m);
		data_queue.push(value);
		cv.notify_one();
	}
	
    bool try_pop(T& value) {
		std::lock_guard<std::mutex> lock(m);
		if (data_queue.empty())
			return false;
		value = data_queue.front();
		data_queue.pop();
		return true;
	}
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, [this] { return !data_queue.empty(); });
		value = data_queue.front();
		data_queue.pop();
	}
    bool empty() const {
		std::lock_guard<std::mutex> lock(m);
		return data_queue.empty();
	}
};

#endif // THREADSAFEQUEUE_H

