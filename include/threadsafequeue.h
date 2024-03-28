#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    
    T data = nullptr;
    int audio_thread_ready = 1;
    int rds_thread_ready = 1;
    int data_ready = 0;
    int data_read[2] = { 0, 0 };
    mutable std::mutex m;
    std::condition_variable consumerCV;
    std::condition_variable producerCV;

public:
    ThreadSafeQueue() {}
    
    void push(const T value) {
		
		std::unique_lock<std::mutex> lock(m);
		
		// Producer waits until both consumer threads are ready
		while (!(audio_thread_ready && rds_thread_ready)) {
			producerCV.wait(lock);
		}
		
		// Once both threads are ready, proceed with data insertion
		if (data) {
			delete data;
		}
		data = value;
		audio_thread_ready = 0;
		rds_thread_ready = 0;
		data_read[0] = 0;
		data_read[1] = 0;
		data_ready = 1;
		consumerCV.notify_all();
	}

    void wait_and_pop(T& value, int indicator) {
        std::unique_lock<std::mutex> lock(m);

        // Wait until data is ready and not read by this consumer thread
		while (!(data_ready && !data_read[indicator])) {
			consumerCV.wait(lock);
		}
		
		// Retrieve data and mark it as read by this thread.
		value = data;
		data_read[indicator] = 1;
		
		// If both threads have read the data, reset data_ready.
		if (data_read[0] && data_read[1]) {
			data_ready = 0;
		}
		lock.unlock();
	}
	
	void prepare(int indicator){
		std::lock_guard<std::mutex> lock(m);
		if (indicator == 1){
			rds_thread_ready = 1;
		}
		if (indicator == 0){
			audio_thread_ready = 1;
		}
		producerCV.notify_all();
	}
	
};

#endif // THREADSAFEQUEUE_H

