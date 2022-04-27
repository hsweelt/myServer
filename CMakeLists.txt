cmake_minimum_required(VERSION 3.22)
project(myServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(myServer main.cpp locker.h threadpool.h locker.cpp)
