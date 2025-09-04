/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the startup guard class to resolve original functions on startup and set up the library.
 */

#include "startup.hpp"

#include <iostream>
#include <dlfcn.h>

#include "overrides.hpp"
#include "config.hpp"
#include "output_timed_queue.hpp"
#include "random_socket_closer.hpp"

CStartup_Guard gStartup_Guard;

CStartup_Guard::CStartup_Guard() {
    orig::socket = reinterpret_cast<int (*)(int, int, int)>(dlsym(RTLD_NEXT, "socket"));
    orig::close = reinterpret_cast<int (*)(int)>(dlsym(RTLD_NEXT, "close"));
    orig::accept = reinterpret_cast<int (*)(int, struct sockaddr*, socklen_t*)>(dlsym(RTLD_NEXT, "accept"));
    orig::recv = reinterpret_cast<ssize_t (*)(int, void*, size_t, int)>(dlsym(RTLD_NEXT, "recv"));
    orig::send = reinterpret_cast<ssize_t (*)(int, const void*, size_t, int)>(dlsym(RTLD_NEXT, "send"));
    orig::read = reinterpret_cast<ssize_t (*)(int, void*, size_t)>(dlsym(RTLD_NEXT, "read"));
    orig::write = reinterpret_cast<ssize_t (*)(int, const void*, size_t)>(dlsym(RTLD_NEXT, "write"));
    orig::shutdown = reinterpret_cast<int (*)(int, int)>(dlsym(RTLD_NEXT, "shutdown"));

    if (!orig::socket) {
        std::cerr << "[[InTCPtor: failed to find original socket() function]]" << std::endl;
    }
    if (!orig::close) {
        std::cerr << "[[InTCPtor: failed to find original close() function]]" << std::endl;
    }
    if (!orig::accept) {
        std::cerr << "[[InTCPtor: failed to find original accept() function]]" << std::endl;
    }
    if (!orig::recv) {
        std::cerr << "[[InTCPtor: failed to find original recv() function]]" << std::endl;
    }
    if (!orig::send) {
        std::cerr << "[[InTCPtor: failed to find original send() function]]" << std::endl;
    }
    if (!orig::read) {
        std::cerr << "[[InTCPtor: failed to find original read() function]]" << std::endl;
    }
    if (!orig::write) {
        std::cerr << "[[InTCPtor: failed to find original write() function]]" << std::endl;
    }
    if (!orig::shutdown) {
        std::cerr << "[[InTCPtor: failed to find original shutdown() function]]" << std::endl;
    }

    // this log is excluded from the conditional, because we always want to know if the library is loaded
    std::cout << "[[InTCPtor: intercepting socket calls]]" << std::endl;

    // initialize all other globals

    // always initialize config first
    gConfig = std::make_unique<CConfig>();

    gOutput_Timed_Queue = std::make_unique<COutput_Timed_Queue>();
    gRandom_Socket_Closer = std::make_unique<CRandom_Socket_Closer>();
}

CStartup_Guard::~CStartup_Guard() {
    std::cout << "[[InTCPtor: stopping intercepting socket calls]]" << std::endl;
}
