cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

project(demo LANGUAGES CXX VERSION 0.0.1)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(code)
