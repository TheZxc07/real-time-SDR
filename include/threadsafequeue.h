#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    
    //std::queue<T> data_queue;
    T data = nullptr;
    int flag = 1;
    mutable std::mutex m;
    bool ready = true;
    std::condition_variable consumerCV;
    std::condition_variable producerCV;

public:
    ThreadSafeQueue() {}
    
    void push(const T value) {
		
		std::unique_lock<std::mutex> lock(m);
		
		if (ready){
			if(data){
				delete data;
			}
			data = value;
			
		} else{
			producerCV.wait(lock, [this] { return ready; });
			if(data){
				delete data;
			}
			data = value;
		}
		ready = false;
		flag = 0;
		//data = value;//data_queue.push(value);
		consumerCV.notify_all();
	}
	/*
    bool try_pop(T value) {
		std::lock_guard<std::mutex> lock(m);
		if (data_queue.empty())
			return false;
		value = data_queue.front();
		data_queue.pop();
		return true;
	}
	*/
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(m);
        //cv.notify_all();
		consumerCV.wait(lock, [this] { return !ready; });
		//flag++;
		value = data;
		//data_queue.pop();
		lock.unlock();
	}
    bool empty() const {
		std::lock_guard<std::mutex> lock(m);
		return data_queue.empty();
	}
	
	void prepare(){
		std::lock_guard<std::mutex> lock(m);
		flag++;
		if (flag == 1){
			ready = true;
		} else{
			ready = false;
		}
		producerCV.notify_all();
	}
	
};

#endif // THREADSAFEQUEUE_H

