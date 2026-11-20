cmake_minimum_required(VERSION 3.15)
project(testllvm)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    # 可用 -DLLVM_PREFIX=... 或 -DLLVM_DIR=... 指定任意 LLVM 14+ 安装路径
    set(LLVM_PREFIX "C:/LLVM/clang+llvm-18.1.8-x86_64-pc-windows-msvc"
        CACHE PATH "LLVM install prefix")
    if(NOT LLVM_DIR)
        set(LLVM_DIR "${LLVM_PREFIX}/lib/cmake/llvm")
    endif()
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Building against LLVM ${LLVM_PACKAGE_VERSION}")
    set(LLVM_INCLUDE_DIRS "${LLVM_PREFIX}/include")
    set(LLVM_LIBRARY_DIRS "${LLVM_PREFIX}/lib")

    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LIBRARY_DIRS})

    llvm_map_components_to_libnames(LLVM_LIBS
            core
            support
            orcjit
            native
    )
    # VS2022 Enterprise 默认 DIA SDK 路径
    set(DIA_SDK_ROOT "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/DIA SDK")

    if(EXISTS "${DIA_SDK_ROOT}/lib/amd64/diaguids.lib")
        # 加入链接目录
        link_directories("${DIA_SDK_ROOT}/lib/amd64")
        message(STATUS "DIA SDK found and linked: ${DIA_SDK_ROOT}/lib/amd64")

        # 可选：显式链接 diaguids.lib（更保险）
        list(APPEND LLVM_LIBS "diaguids.lib")
    else()
        message(WARNING "DIA SDK not found at ${DIA_SDK_ROOT}")
        message(WARNING "  → If you don't need PDB, replace 'native' with 'nativecodegen'")
    endif()
    # 添加可执行文件
    add_executable(testllvm src/main.cpp)
    add_executable(testpass src/testpass.cpp)
    add_executable(testpass2 src/testpass2.cpp)
    add_executable(adt_string src/adt_string.cpp)
    add_executable(adt src/adt.cpp)

    # 链接 LLVM 库
    target_link_libraries(testllvm ${LLVM_LIBS})
    target_link_libraries(testpass ${LLVM_LIBS})
    target_link_libraries(testpass2 ${LLVM_LIBS})
    target_link_libraries(adt_string ${LLVM_LIBS})
    target_link_libraries(adt ${LLVM_LIBS})

    # Windows 特定链接选项
    target_link_libraries(testllvm advapi32 shell32)
    target_link_libraries(testpass advapi32 shell32)
    target_link_libraries(testpass2 advapi32 shell32)
    target_link_libraries(adt_string advapi32 shell32)
    target_link_libraries(adt advapi32 shell32)

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

    # 链接 LLVM 库（推荐使用组件）
    llvm_map_components_to_libnames(llvm_libs
            core
            executionengine
            orcjit
            native
            support
    )

    add_executable(testllvm src/main.cpp)
    add_executable(testpass src/testpass.cpp)
    add_executable(testpass2 src/testpass2.cpp)
    add_executable(adt_string src/adt_string.cpp)
    add_executable(adt src/adt.cpp)

    target_link_libraries(testllvm ${llvm_libs})
    target_link_libraries(testpass ${llvm_libs})
    target_link_libraries(testpass2 ${llvm_libs})
    target_link_libraries(adt_string ${llvm_libs})
    target_link_libraries(adt ${llvm_libs})
endif()