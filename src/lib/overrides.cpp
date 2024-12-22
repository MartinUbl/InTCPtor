/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 * 
 * This library is designed to be used with the runner program or directly with the LD_PRELOAD environment variable.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <set>
#include <random>
#include <thread>
#include <chrono>
#include <mutex>

namespace {

    // random number generators to generate trouble non-deterministically
    std::random_device rdev;
    std::mt19937 reng(rdev());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // managed sockets
    std::set<int> _created_sockets;
    std::set<int> _managed_sockets;

    // global mutex to prevent race conditions during simulated network trouble
    // normally, the system handles this, but as we are simulating network trouble, we need to ensure that we don't have multiple threads interfering with each other
    // the mutex is recursive, as we may call read() from recv(), but at the same time, other threads may call either of those functions directly, so we need to protect
    // both of them with a mutex
    std::recursive_mutex _glob_mutex;
}

// original socket-related functions
namespace orig {
    int (*socket)(int, int, int) = nullptr;
    int (*close)(int) = nullptr;
    int (*accept)(int, struct sockaddr*, socklen_t*) = nullptr;
    ssize_t (*recv)(int, void*, size_t, int) = nullptr;
    ssize_t (*send)(int, const void*, size_t, int) = nullptr;
    ssize_t (*read)(int, void*, size_t) = nullptr;
    ssize_t (*write)(int, const void*, size_t) = nullptr;
}

// guard class to resolve original functions on startup
class CStartup_Guard {
    public:
        CStartup_Guard() {
            orig::socket = reinterpret_cast<int (*)(int, int, int)>(dlsym(RTLD_NEXT, "socket"));
            orig::close = reinterpret_cast<int (*)(int)>(dlsym(RTLD_NEXT, "close"));
            orig::accept = reinterpret_cast<int (*)(int, struct sockaddr*, socklen_t*)>(dlsym(RTLD_NEXT, "accept"));
            orig::recv = reinterpret_cast<ssize_t (*)(int, void*, size_t, int)>(dlsym(RTLD_NEXT, "recv"));
            orig::send = reinterpret_cast<ssize_t (*)(int, const void*, size_t, int)>(dlsym(RTLD_NEXT, "send"));
            orig::read = reinterpret_cast<ssize_t (*)(int, void*, size_t)>(dlsym(RTLD_NEXT, "read"));
            orig::write = reinterpret_cast<ssize_t (*)(int, const void*, size_t)>(dlsym(RTLD_NEXT, "write"));

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

            std::cout << "[[InTCPtor: intercepting socket calls]]" << std::endl;
        }

        ~CStartup_Guard() {
            std::cout << "[[InTCPtor: stopping intercepting socket calls]]" << std::endl;
        }
};

// global guard instance to resolve original functions on startup
CStartup_Guard _startup_guard;

// override socket() to track created sockets
extern "C" int socket(int domain, int type, int protocol) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    int res = orig::socket(domain, type, protocol);

    std::cout << "[[InTCPtor: overriden socket() call, result = " << res << "]]" << std::endl;

    _created_sockets.insert(res);

    return res;
}

// override close() to track closed sockets
extern "C" int close(int fd) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    if (_created_sockets.find(fd) != _created_sockets.end()) {
        std::cout << "[[InTCPtor: overriden close() call for server socket fd = " << fd << "]]" << std::endl;
        _created_sockets.erase(fd);
    }
    else if (_managed_sockets.find(fd) != _managed_sockets.end()) {
        std::cout << "[[InTCPtor: overriden close() call for client socket fd = " << fd << "]]" << std::endl;
        _managed_sockets.erase(fd);
    }
    else {
        std::cout << "[[InTCPtor: overriden close() call for non-managed fd = " << fd << "]]" << std::endl;
    }

    return orig::close(fd);
}

// override accept() to track accepted sockets
extern "C" int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    int res = orig::accept(sockfd, addr, addrlen);

    std::cout << "[[InTCPtor: overriden accept() call, result = " << res << "]]" << std::endl;

    _managed_sockets.insert(res);

    return res;
}

// override recv() to simulate network trouble
extern "C" ssize_t recv(int sockfd, void *buf, size_t count, int flags) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    if (count > 2) {

        const double chance = dist(reng);

        if (chance > 0.3) {
            const size_t orig = count;
            if (chance > 0.9) {
                count -= 1;
            }
            else if (chance > 0.6) {
                count /= 2;
            }
            else if (chance > 0.4) {
                count = 2;
            }
            else {
                count -= 2;
            }

            std::cout << "[[InTCPtor: recv() original count = " << orig << ", adjusted = " << count << "]]" << std::endl;
        }
    }

    ssize_t res = orig::recv(sockfd, buf, count, flags);

    std::cout << "[[InTCPtor: overriden recv() call, result = " << res << "]]" << std::endl;

    return res;
}

// override send() to simulate network trouble
extern "C" ssize_t send(int sockfd, const void *buf, size_t count, int flags) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    ssize_t res = 0;
    
    auto adjusted_send = [&](size_t offset, size_t lcount) {

        if (offset + lcount > count) {
            lcount = count - offset;
        }

        ssize_t localres = orig::send(sockfd, reinterpret_cast<const char*>(buf) + offset, lcount, flags);

        res += localres;

        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(100 + dist(reng) * 300)));
    };

    bool adjusted = false;
    if (count > 2) {

        const double chance = dist(reng);

        if (chance > 0.3) {
            adjusted = true;

            if (chance > 0.9) {
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 1B sends]]" << std::endl;
                for (size_t i = 0; i < count; i++) {
                    adjusted_send(i, 1);
                }
            }
            else if (chance > 0.6) {
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2 separate sends]]" << std::endl;
                const size_t half = count / 2;
                adjusted_send(0, half);
                adjusted_send(half, count - half);
            }
            else if (chance > 0.4) {
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2B sends and second send]]" << std::endl;
                adjusted_send(0, 2);
                adjusted_send(2, count - 2);
            }
            else {
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2B sends]]" << std::endl;
                for (size_t i = 0; i < count; i += 2) {
                    adjusted_send(i, 2);
                }
            }
        }
    }

    if (!adjusted) {
        ssize_t res = orig::send(sockfd, buf, count, flags);

        std::cout << "[[InTCPtor: overriden send() call, result = " << res << "]]" << std::endl;
    }

    return res;
}

// override read() to simulate network trouble
extern "C" ssize_t read(int fd, void *buf, size_t count) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    if (_managed_sockets.find(fd) == _managed_sockets.end() && _created_sockets.find(fd) == _created_sockets.end()) {
        return orig::read(fd, buf, count);
    }

    std::cout << "[[InTCPtor: override read() as recv() with flags = 0]]" << std::endl;

    return recv(fd, buf, count, 0);
}

// override write() to simulate network trouble
extern "C" ssize_t write(int fd, const void *buf, size_t count) {

    std::unique_lock<std::recursive_mutex> lock(_glob_mutex);

    if (_managed_sockets.find(fd) == _managed_sockets.end() && _created_sockets.find(fd) == _created_sockets.end()) {
        return orig::write(fd, buf, count);
    }

    std::cout << "[[InTCPtor: override write() as send() with flags = 0]]" << std::endl;

    return send(fd, buf, count, 0);
}
