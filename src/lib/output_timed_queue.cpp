/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains output timed queue to properly delay sending data to sockets.
 */

#include "output_timed_queue.hpp"

#include <iostream>

#include "overrides.hpp"

COutput_Timed_Queue::TPtr gOutput_Timed_Queue;

COutput_Timed_Queue::COutput_Timed_Queue() {
    std::cout << "[[InTCPtor: starting output timed queue]]" << std::endl;
    _running = true;
    _worker = std::thread(&COutput_Timed_Queue::worker, this);
}

COutput_Timed_Queue::~COutput_Timed_Queue() {
    _running = false;
    _worker.join();
}

void COutput_Timed_Queue::push(int target_socket, size_t delay, const char* data, size_t len) {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push({target_socket, delay, std::vector<char>(data, data + len)});
    _cond.notify_one();
}

void COutput_Timed_Queue::worker() {
    while (_running) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait_for(lock, std::chrono::milliseconds(100), [this] { return !_queue.empty() || !_running; });

        while (!_queue.empty()) {
            const TOut_Data& data = _queue.front();
            lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(data.delay)));

            orig::send(data.target_socket, data.data.data(), data.data.size(), 0);

            lock.lock();

            _queue.pop();
        }
    }
}
