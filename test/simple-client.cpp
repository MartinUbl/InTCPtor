/*
 * InTCPtor - example simple TCP client
 *
 * This file contains a very simple TCP client to test the InTCPtor library.
 * Messages start with "ABCD" and end with "\n"; the client performs very simple continual send/receive loop to ensure
 * messages are properly sent and received as a whole.
 * 
 * WARNING: this is just a simple example, not a robust client implementation!
 */

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

const std::string Default_Addr = "127.0.0.1";
const int Default_Port = 10000;

int main(int argc, char** argv) {

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
	my_addr.sin_port = htons(Default_Port);
	my_addr.sin_addr.s_addr = inet_addr(Default_Addr.c_str());

	return_value = connect(client_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
	if (return_value != 0)  {
		return 2;
	}

	for (size_t ii = 0; ii < 10; ii++) {

		// send some messages starting with expected header and ending with newline
		send(client_socket, "ABCDtest\n", 9, 0);
		send(client_socket, "ABCDtesttest\n", 13, 0);

		char buf[128];
		memset(buf, 0, 128);

		// very simple receive loop until newline - receive bytes one by one
		// WARNING: this is just a simple example, not a robust client implementation!
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
