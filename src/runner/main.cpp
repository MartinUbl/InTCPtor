/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file is just a convenience runner for the InTCPtor library. It hooks the library into the execution of a binary using LD_PRELOAD.
 */

#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <cstring>
#include <cerrno>

int main(int argc, char** argv) {

	std::cout << "[[InTCPtor Runner: starting]]" << std::endl;

	if (argc < 2) {
		std::cerr << "[[InTCPtor Runner: usage: " << argv[0] << " <path to the binary to run> [<optional arguments>] ]]" << std::endl;
		return 1;
	}

	std::cout << "[[InTCPtor Runner: executing " << argv[1] << "]]" << std::endl;
	if (argc > 2) {
		std::cout << "[[InTCPtor Runner: arguments: ";
		for (int i = 2; i < argc; i++) {
			std::cout << "'" << argv[i] << "' ";
		}
		std::cout << "]]" << std::endl;
	}

	std::string basePath = "./";

	// get current executable absolute path
	std::string buf(1024, '\0');
	if (readlink("/proc/self/exe", buf.data(), 1024) > 0) {
		// get the directory of the executable using std::filesystem
		basePath = std::filesystem::path(buf).parent_path().string();

		std::cout << "[[InTCPtor Runner: using resolved base path: " << basePath << "]]" << std::endl;
	}
	else {
		std::cerr << "[[InTCPtor Runner: error: cannot determine the base path, using current working directory]]" << std::endl;
	}

	// append / to the base path if it doesn't end with it
	if (basePath.length() > 0 && basePath.back() != '/') {
		basePath += "/";
	}

	const auto libPath = basePath + "libintcptor-overrides.so";

	// check if the libintcptor-overrides.so file exists
	if (!std::filesystem::exists(libPath)) {
		std::cerr << "[[InTCPtor Runner: error: libintcptor-overrides.so not found in the requested path: " << basePath << "]]" << std::endl;
		return 1;
	}

	// check if the binary to run exists
	if (!std::filesystem::exists(argv[1])) {
		std::cerr << "[[InTCPtor Runner: error: executable file '" << argv[1] << "' not found]]" << std::endl;
		return 1;
	}

	// Set LD_PRELOAD to the path of the shared library
	// This will be used to intercept the socket calls
	if (setenv("LD_PRELOAD", libPath.c_str(), 1) != 0) {
		std::cerr << "[[InTCPtor Runner: error: cannot set LD_PRELOAD environment variable, errno = " << errno << "]]" << std::endl;
		return 1;
	}

	// Execute the binary with the given arguments
	int res = execv(argv[1], argv + 1);
	if (res == -1) {
		std::cerr << "[[InTCPtor Runner: error: cannot execute the requested binary, errno = " << errno << "]]" << std::endl;
		return 1;
	}

	return 0;
}
