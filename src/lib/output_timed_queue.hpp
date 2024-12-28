/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains output timed queue to properly delay sending data to sockets.
 */

#pragma once

#include <thread>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <vector>
#include <memory>

class COutput_Timed_Queue {
    public:
        using TPtr = std::unique_ptr<COutput_Timed_Queue>;

        COutput_Timed_Queue();

        virtual ~COutput_Timed_Queue();

        void push(int target_socket, size_t delay, const char* data, size_t len);

    private:
        void worker();

        struct TOut_Data {
            int target_socket;
            size_t delay;
            std::vector<char> data;
        };

        std::thread _worker;
        std::mutex _mutex;
        std::condition_variable _cond;
        std::queue<TOut_Data> _queue;
        bool _running = true;
};

extern COutput_Timed_Queue::TPtr gOutput_Timed_Queue;
