cmake_minimum_required(VERSION 2.8)
project(json)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Boost 1.54 REQUIRED)
include_directories(${Boost_INCLUDE_DIR} include)

enable_testing()
add_subdirectory(test)
