cmake_minimum_required(VERSION 3.29)
project(RaftStore CXX)
set(CMAKE_CXX_STANDARD 11)

include_directories(src/logger/include)
add_executable(log src/main.cxx src/logger/src/Logger.cxx)