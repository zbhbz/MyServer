cmake_minimum_required(VERSION 2.8)

project(MyServer)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_CXX_FLAGS  "-g -Wall -std=c++11 -pthread -O3" )

add_subdirectory(Server)
