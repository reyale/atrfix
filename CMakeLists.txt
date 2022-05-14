cmake_minimum_required(VERSION 3.10)

project(atrfix)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(server example/server.cpp)
add_executable(client example/client.cpp)
