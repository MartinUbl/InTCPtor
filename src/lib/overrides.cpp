/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the implementation of the socket-related functions that are being intercepted.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <mutex>

#include "overrides.hpp"
#include "config.hpp"

#include "output_timed_queue.hpp"

// original socket-related functions
namespace orig {
    int (*socket)(int, int, int) = nullptr;
    int (*close)(int) = nullptr;
    int (*accept)(int, struct sockaddr*, socklen_t*) = nullptr;
    ssize_t (*recv)(int, void*, size_t, int) = nullptr;
    ssize_t (*send)(int, const void*, size_t, int) = nullptr;
    ssize_t (*read)(int, void*, size_t) = nullptr;
    ssize_t (*write)(int, const void*, size_t) = nullptr;
    int (*shutdown)(int, int) = nullptr;
}

namespace intcptor {
    std::set<int> created_sockets;
    std::set<int> managed_sockets;
    std::recursive_mutex glob_mutex;
}

// override socket() to track created sockets
extern "C" int socket(int domain, int type, int protocol) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    int res = orig::socket(domain, type, protocol);

    std::cout << "[[InTCPtor: overriden socket() call, result = " << res << "]]" << std::endl;

    intcptor::created_sockets.insert(res);

    return res;
}

// override close() to track closed sockets
extern "C" int close(int fd) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    if (intcptor::created_sockets.find(fd) != intcptor::created_sockets.end()) {
        std::cout << "[[InTCPtor: overriden close() call for server socket fd = " << fd << "]]" << std::endl;
        intcptor::created_sockets.erase(fd);
    }
    else if (intcptor::managed_sockets.find(fd) != intcptor::managed_sockets.end()) {
        std::cout << "[[InTCPtor: overriden close() call for client socket fd = " << fd << "]]" << std::endl;
        intcptor::managed_sockets.erase(fd);
    }
    else {
        std::cout << "[[InTCPtor: overriden close() call for non-managed fd = " << fd << "]]" << std::endl;
    }

    return orig::close(fd);
}

// override accept() to track accepted sockets
extern "C" int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {

    //std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);
    // not locking here, as accept call may block

    int res = orig::accept(sockfd, addr, addrlen);

    std::cout << "[[InTCPtor: overriden accept() call, result = " << res << "]]" << std::endl;

    intcptor::managed_sockets.insert(res);

    return res;
}

// override recv() to simulate network trouble
extern "C" ssize_t recv(int sockfd, void *buf, size_t count, int flags) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    if (count > 2) {

        const double chance = gConfig->Generate_Base_Prob();

        if (chance < gConfig->GetProb_Recv_Total()) {
            const size_t orig = count;
            if (chance < gConfig->GetProb_Recv__1B_Less()) {
                count -= 1;
            }
            else if (chance < gConfig->GetProb_Recv__1B_Less() + gConfig->GetProb_Recv__2B_Less()) {
                count /= 2;
            }
            else if (chance < gConfig->GetProb_Recv__1B_Less() + gConfig->GetProb_Recv__2B_Less() + gConfig->GetProb_Recv__Half()) {
                count = 2;
            }
            else {
                count -= 2;
            }

            std::cout << "[[InTCPtor: recv() original count = " << orig << ", adjusted = " << count << "]]" << std::endl;
        }
    }

    // no longer needed
    lock.unlock();

    ssize_t res = orig::recv(sockfd, buf, count, flags);

    std::cout << "[[InTCPtor: overriden recv() call, result = " << res << "]]" << std::endl;

    return res;
}

// override send() to simulate network trouble
extern "C" ssize_t send(int sockfd, const void *buf, size_t count, int flags) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    ssize_t res = 0;
    
    auto adjusted_send = [&](size_t offset, size_t lcount) {

        if (offset + lcount > count) {
            lcount = count - offset;
        }

        res += lcount;

        gOutput_Timed_Queue->push(sockfd, gConfig->Generate_Send_Delay(), reinterpret_cast<const char*>(buf) + offset, lcount);
    };

    bool adjusted = false;
    if (count > 2) {

        const double chance = gConfig->Generate_Base_Prob();

        if (chance < gConfig->GetProb_Send_Total()) {
            adjusted = true;

            if (chance < gConfig->GetProb_Send__1B_Sends()) {
                for (size_t i = 0; i < count; i++) {
                    adjusted_send(i, 1);
                }
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 1B sends]]" << std::endl;
            }
            else if (chance < gConfig->GetProb_Send__1B_Sends() + gConfig->GetProb_Send__2_Separate_Sends()) {
                const size_t half = count / 2;
                adjusted_send(0, half);
                adjusted_send(half, count - half);
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2 separate sends]]" << std::endl;
            }
            else if (chance < gConfig->GetProb_Send__1B_Sends() + gConfig->GetProb_Send__2_Separate_Sends() + gConfig->GetProb_Send__2B_Sends_And_Second_Send()) {
                adjusted_send(0, 2);
                adjusted_send(2, count - 2);
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2B sends and second send]]" << std::endl;
            }
            else {
                for (size_t i = 0; i < count; i += 2) {
                    adjusted_send(i, 2);
                }
                std::cout << "[[InTCPtor: send() original count = " << count << ", adjusted to 2B sends]]" << std::endl;
            }
        }
    }

    // no longer needed
    lock.unlock();

    if (!adjusted) {
        ssize_t res = orig::send(sockfd, buf, count, flags);

        std::cout << "[[InTCPtor: overriden send() call, result = " << res << "]]" << std::endl;
    }

    return res;
}

// override read() to simulate network trouble
extern "C" ssize_t read(int fd, void *buf, size_t count) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    if (intcptor::managed_sockets.find(fd) == intcptor::managed_sockets.end() && intcptor::created_sockets.find(fd) == intcptor::created_sockets.end()) {
        return orig::read(fd, buf, count);
    }

    std::cout << "[[InTCPtor: override read() as recv() with flags = 0]]" << std::endl;

    return recv(fd, buf, count, 0);
}

// override write() to simulate network trouble
extern "C" ssize_t write(int fd, const void *buf, size_t count) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    if (intcptor::managed_sockets.find(fd) == intcptor::managed_sockets.end() && intcptor::created_sockets.find(fd) == intcptor::created_sockets.end()) {
        return orig::write(fd, buf, count);
    }

    std::cout << "[[InTCPtor: override write() as send() with flags = 0]]" << std::endl;

    return send(fd, buf, count, 0);
}

// override shutdown() to track closed sockets
extern "C" int shutdown(int sockfd, int how) {

    std::unique_lock<std::recursive_mutex> lock(intcptor::glob_mutex);

    // NOTE: we don't track shutdowns, as they are not always followed by close() calls
    // furthermore, we should wait here until all sent data is actually sent, so we can't close the socket immediately
    // this is a TODO for future work, but may not be actually needed

    if (intcptor::created_sockets.find(sockfd) != intcptor::created_sockets.end()) {
        std::cout << "[[InTCPtor: overriden shutdown() call for server socket fd = " << sockfd << "]]" << std::endl;
    }
    else if (intcptor::managed_sockets.find(sockfd) != intcptor::managed_sockets.end()) {
        std::cout << "[[InTCPtor: overriden shutdown() call for client socket fd = " << sockfd << "]]" << std::endl;
    }
    else {
        std::cout << "[[InTCPtor: overriden shutdown() call for non-managed fd = " << sockfd << "]]" << std::endl;
    }

    return orig::shutdown(sockfd, how);
}
