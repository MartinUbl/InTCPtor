/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the implementation of the random socket closer.
 */

#include "random_socket_closer.hpp"

#include "overrides.hpp"
#include "config.hpp"

#include <iostream>

CRandom_Socket_Closer::TPtr gRandom_Socket_Closer;

CRandom_Socket_Closer::CRandom_Socket_Closer() {

    if (!gConfig->Should_Drop_Connections()) {
        std::cout << "[[InTCPtor: not dropping connections]]" << std::endl;
        _running = false;
        return;
    }

    std::cout << "[[InTCPtor: will randomly drop connections with delay between " << gConfig->GetDrop_Connection_Delay_Ms_Min() << " and " << gConfig->GetDrop_Connection_Delay_Ms_Max() << " ms]]" << std::endl;
    _running = true;
    _worker = std::thread(&CRandom_Socket_Closer::worker, this);
}

CRandom_Socket_Closer::~CRandom_Socket_Closer() {
    _running = false;
    if (_worker.joinable()) {
        _worker.join();
    }
}

void CRandom_Socket_Closer::worker() {
    while (_running) {
        std::unique_lock<std::mutex> lock(_mutex);

        auto delay = std::chrono::milliseconds(static_cast<int>(gConfig->GetDrop_Connection_Delay_Ms_Min() + gConfig->Generate_Base_Prob() * (gConfig->GetDrop_Connection_Delay_Ms_Max() - gConfig->GetDrop_Connection_Delay_Ms_Min())));

        // we actually don't care about spurious/stolen wakeups
        _cond.wait_for(lock, std::chrono::milliseconds(delay));

        // randomly close only accepted client sockets
        if (intcptor::managed_sockets.empty()) {
            continue;
        }

        auto it = intcptor::managed_sockets.begin();
        std::advance(it, static_cast<int>(gConfig->Generate_Base_Prob() * intcptor::managed_sockets.size()));

        if (it != intcptor::managed_sockets.end()) {

            std::cout << "[[InTCPtor: closing random client socket: " << *it << "]]" << std::endl;

            // it is important to call shutdown, to block further transmission on the socket
            orig::shutdown(*it, SHUT_RDWR);
            orig::close(*it);
            intcptor::managed_sockets.erase(it);
        }
    }
}
