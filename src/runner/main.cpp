#include <iostream>
#include <filesystem>
#include <unistd.h>

int main(int argc, char** argv) {

	std::cout << "InTCPtor Runner" << std::endl;

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <path to the binary to run>" << std::endl;
		return 1;
	}

	std::cout << "> Executing: " << argv[1] << std::endl;
	if (argc > 2) {
		std::cout << "> Arguments: ";
		for (int i = 2; i < argc; i++) {
			std::cout << "'" << argv[i] << "' ";
		}
		std::cout << std::endl << std::endl;
	}

	// check if the libintcptor-overrides.so file exists
	if (!std::filesystem::exists("./libintcptor-overrides.so")) {
		std::cerr << "Error: libintcptor-overrides.so not found" << std::endl;
		return 1;
	}

	// check if the binary to run exists
	if (!std::filesystem::exists(argv[1])) {
		std::cerr << "Error: executable file '" << argv[1] << "' not found" << std::endl;
		return 1;
	}

	// Set LD_PRELOAD to the path of the shared library
	// This will be used to intercept the socket calls
	setenv("LD_PRELOAD", "./libintcptor-overrides.so", 1);

	// Execute the binary with the given arguments
	int res = execv(argv[1], argv + 1);
	if (res == -1) {
		std::cerr << "Error: cannot execute the requested binary" << std::endl;
		return 1;
	}

	return 0;
}
