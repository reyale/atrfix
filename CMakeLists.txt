cmake_minimum_required(VERSION 3.10)

project(atrfix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

add_definitions(-DFMT_HEADER_ONLY=1)

include_directories(include thirdparty)

find_package(Boost COMPONENTS thread REQUIRED)

# add the executable
add_executable(server example/server.cpp)
add_executable(client example/client.cpp)
add_executable(test example/test.cpp)

target_link_libraries(
    client 
    Boost::thread
  )
