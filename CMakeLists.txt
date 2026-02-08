cmake_minimum_required(VERSION 3.30)
project(A3)

set(CMAKE_CXX_STANDARD 20)

add_executable(A3 main.cpp hash_func_gen.cpp hyperloglog.cpp random_stream_gen.cpp)
