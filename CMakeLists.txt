cmake_minimum_required(VERSION 3.15)
project(testllvm)

set(CMAKE_CXX_STANDARD 17)

# Windows 特定设置
if(WIN32)
    # 手动设置路径
    set(LLVM_PREFIX "C:/LLVM")
    set(LLVM_INCLUDE_DIRS "${LLVM_PREFIX}/include")
    set(LLVM_LIBRARY_DIRS "${LLVM_PREFIX}/lib")

    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LIBRARY_DIRS})
else ()
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
    add_definitions(${LLVM_DEFINITIONS})
    # 链接 LLVM 库（推荐使用组件）
    llvm_map_components_to_libnames(llvm_libs
            core
            executionengine
            orcjit
            native
            support
    )

endif()
# 可执行文件

add_executable(testllvm src/main.cpp)
add_executable(testpass src/testpass.cpp)
add_executable(testpass2 src/testpass2.cpp)
# 链接 LLVM
target_link_libraries(testllvm ${llvm_libs})
target_link_libraries(testpass ${llvm_libs})
target_link_libraries(testpass2 ${llvm_libs})

# Windows 特定设置
if(WIN32)
    # 添加 Windows 特定的链接选项
    target_link_libraries(testllvm advapi32 shell32)
    target_link_libraries(testpass advapi32 shell32)

    # 设置字符集
    add_compile_definitions(UNICODE _UNICODE)

    # 如果是 Debug 构建，链接调试库
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(testllvm debug LLVM-C)
        target_link_libraries(testpass debug LLVM-C)
    endif()
endif()