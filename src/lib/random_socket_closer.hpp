/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the implementation of the random socket closer.
 */

#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>

class CRandom_Socket_Closer {
    public:
        using TPtr = std::unique_ptr<CRandom_Socket_Closer>;

        CRandom_Socket_Closer();

        virtual ~CRandom_Socket_Closer();

    private:
        void worker();

        std::thread _worker;
        std::mutex _mutex;
        std::condition_variable _cond;
        bool _running = true;
};

extern CRandom_Socket_Closer::TPtr gRandom_Socket_Closer;
