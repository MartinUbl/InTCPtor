#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <thread>
#include <chrono>

int main(int argc, char** argv) {

	// store IP address and port number from argv
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <IP address> <port number>" << std::endl;
		return 1;
	}

	// store IP address to string
	std::string ip_addr = argv[1];
	// store port
	int port = atoi(argv[2]);

	if (port < 0 || port > 65535) {
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}

	int client_socket;
	int return_value;
	char cbuf;
	int len_addr;
	struct sockaddr_in my_addr;

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (client_socket <= 0) {
		return 1;
	}

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());

	return_value = connect(client_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
	if (return_value != 0)  {
		return 2;
	}

	for (size_t ii = 0; ii < 10; ii++) {

		send(client_socket, "ABCDtest\n", 9, 0);
		send(client_socket, "ABCDtesttest\n", 13, 0);

		char buf[128];
		memset(buf, 0, 128);

		size_t pos = 0;
		do {
			recv(client_socket, buf + pos, 1, 0);
		} while (buf[pos] != '\n' && ++pos < 128);

		std::cout << "Received: " << buf << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	
	close(client_socket);

	return 0;
}
