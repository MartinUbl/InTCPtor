#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <vector>
#include <set>

const std::string Hdr_Magic{ "ABCD" };

int main(int argc, char** argv) {

    int port = 10000;
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    if (port < 0 || port > 65535) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    std::string server_bind_addr = "127.0.0.1";
    if (argc > 1) {
        server_bind_addr = argv[1];
    }

    if (server_bind_addr.empty()) {
        std::cerr << "Invalid IP address" << std::endl;
        return 1;
    }

    int server_fd, new_socket, max_sd, valread, sd;
    std::set<int> client_socket;
    struct sockaddr_in address;
    fd_set readfds;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Could not initialize socket" << std::endl;
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Could not set socket SO_REUSEADDR option" << std::endl;
        return -1;
    }

    // Set server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_bind_addr.c_str());
    address.sin_port = htons(port);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Could not bind socket to address " << server_bind_addr << ":" << port << std::endl;
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Could not listen on socket" << std::endl;
        return -1;
    }

    std::cout << "Listening on " << server_bind_addr << ":" << port << std::endl;

    while (true) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add child sockets to set
        for (auto skt : client_socket) {
            if (skt > 0) {
                FD_SET(skt, &readfds);
            }

            if (skt > max_sd) {
                max_sd = skt;
            }
        }

        int res = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (res < 0) {
            std::cerr << "Select reported an error: " << errno << std::endl;
            return -2;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            std::cout << "New connection from: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;

            client_socket.insert(new_socket);
        }

        std::set<int> to_remove;
        std::vector<char> buffer(128);

        // Check all clients for incoming data
        for (auto sd : client_socket) {

            if (FD_ISSET(sd, &readfds)) {

                res = recv(sd, buffer.data(), static_cast<int>(buffer.size()), 0);

                if (res <= 0) {
                    to_remove.insert(sd);
                } else {
                    if (std::string_view(buffer.data(), Hdr_Magic.size()) == Hdr_Magic) {

                        // parse until newline
                        int msg_len = 0;
                        for (int i = Hdr_Magic.size(); i < res; i++) {
                            if (buffer[i] == '\n') {
                                msg_len = i - Hdr_Magic.size();
                                break;
                            }
                        }

                        std::string message(buffer.data() + Hdr_Magic.size(), msg_len);
                        std::cout << "Received message: " << message << std::endl;

                        send(sd, "ABCDhellothere\n", 15, 0);
                    } else { 
                        std::cerr << "Invalid message format" << std::endl;
                        //to_remove.insert(sd);
                    }
                }
            }
        }

        // Remove disconnected clients
        for (auto sd : to_remove) {
            close(sd);
            client_socket.erase(sd);
        }
    }

    return 0;
}