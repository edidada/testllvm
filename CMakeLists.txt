cmake_minimum_required(VERSION 3.15)
project(testllvm)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")
# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(/usr/include/llvm-14)
add_definitions(${LLVM_DEFINITIONS})

add_executable(testllvm src/main.cpp)
target_link_libraries(testllvm llvm-core::llvm-core)
