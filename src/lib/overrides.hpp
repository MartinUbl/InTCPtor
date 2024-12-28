/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the implementation of the socket-related functions that are being intercepted.
 */

#pragma once

#include <arpa/inet.h>
#include <unistd.h>

// original socket-related functions
namespace orig {
    extern int (*socket)(int, int, int);
    extern int (*close)(int);
    extern int (*accept)(int, struct sockaddr*, socklen_t*);
    extern ssize_t (*recv)(int, void*, size_t, int);
    extern ssize_t (*send)(int, const void*, size_t, int);
    extern ssize_t (*read)(int, void*, size_t);
    extern ssize_t (*write)(int, const void*, size_t);
    extern int (*shutdown)(int, int);
}

#include <set>
#include <mutex>

namespace intcptor {
    // created sockets (via socket() call)
    extern std::set<int> created_sockets;
    // managed sockets (accepted sockets via accept() call)
    extern std::set<int> managed_sockets;

    // global mutex to prevent race conditions during simulated network trouble
    // normally, the system handles this, but as we are simulating network trouble, we need to ensure that we don't have multiple threads interfering with each other
    // the mutex is recursive, as we may call read() from recv(), but at the same time, other threads may call either of those functions directly, so we need to protect
    // both of them with a mutex
    extern std::recursive_mutex glob_mutex;
}
