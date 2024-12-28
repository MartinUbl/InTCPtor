CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(InTCPtor)

ADD_EXECUTABLE(intcptor-run src/runner/main.cpp)
ADD_LIBRARY(intcptor-overrides SHARED src/lib/overrides.cpp src/lib/config.cpp src/lib/config.hpp src/lib/output_timed_queue.cpp src/lib/startup.cpp src/lib/random_socket_closer.cpp)

ADD_EXECUTABLE(intcptor-test-simple-client test/simple-client.cpp)
ADD_EXECUTABLE(intcptor-test-simple-server test/simple-server.cpp)

TARGET_LINK_LIBRARIES(intcptor-run dl)
TARGET_LINK_LIBRARIES(intcptor-overrides dl)
