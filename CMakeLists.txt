cmake_minimum_required(VERSION 3.15)
project(testllvm)

set(CMAKE_CXX_STANDARD 17)

# Windows 特定设置
if(WIN32)
    # 手动设置路径
    set(LLVM_DIR "C:/LLVM/clang+llvm-18.1.8-x86_64-pc-windows-msvc/lib/cmake/llvm")
    find_package(LLVM REQUIRED CONFIG)
    set(LLVM_PREFIX "C:/LLVM/clang+llvm-18.1.8-x86_64-pc-windows-msvc")
    set(LLVM_INCLUDE_DIRS "${LLVM_PREFIX}/include")
    set(LLVM_LIBRARY_DIRS "${LLVM_PREFIX}/lib")

    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LIBRARY_DIRS})

    llvm_map_components_to_libnames(LLVM_LIBS
            core
            support
            orcjit
            nativecodegen
    )
    # 添加可执行文件
    add_executable(testllvm src/main.cpp)
    add_executable(testpass src/testpass.cpp)
    add_executable(testpass2 src/testpass2.cpp)

    # 链接 LLVM 库
    target_link_libraries(testllvm ${LLVM_LIBS})
    target_link_libraries(testpass ${LLVM_LIBS})
    target_link_libraries(testpass2 ${LLVM_LIBS})

    # Windows 特定链接选项
    target_link_libraries(testllvm advapi32 shell32)
    target_link_libraries(testpass advapi32 shell32)
    target_link_libraries(testpass2 advapi32 shell32)

else()
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

    # 自动映射组件
    llvm_map_components_to_libnames(llvm_libs
            core support executionengine orcjit orctargetprocess runtimedyld
            mc mcparser object bitreader bitwriter transformutils analysis
            target x86codegen x86desc x86info
    )

    add_executable(testllvm src/main.cpp)
    add_library(testpass MODULE src/testpass.cpp)
    add_library(testpass2 MODULE src/testpass2.cpp)

    target_link_libraries(testllvm ${llvm_libs})
    target_link_libraries(testpass ${llvm_libs})
    target_link_libraries(testpass2 ${llvm_libs})
endif()