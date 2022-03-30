cmake_minimum_required(VERSION 3.4...3.18)
project(quandelibc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

EXEC_PROGRAM("/usr/sbin/sysctl -n hw.optional.arm64" OUTPUT_VARIABLE IS_ARM64)

if (MSVC)
    add_compile_options(/arch:AVX2 /MT /Zc:__cplusplus)
else (MSVC)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
    # used for SIMD optimization
    if (NOT ${IS_ARM64})
        # disable AVX optimization on Apple ARM64
        add_compile_options(-mavx)
    endif ()
endif (MSVC)

find_package(Threads)

set(QLIBC_SOURCES
        src/fockstate.cpp src/fockstate.h
        src/fs_array.cpp src/fs_array.h
        src/fs_map.cpp src/fs_map.h
        src/fs_mask.cpp
        src/memory_tools.h
        src/optmul.h
        src/permanent.h
        src/permanent_glynn.h
        src/permanent_ryser.h
        src/sub_permanents.h)

add_subdirectory(extern/pybind11)
pybind11_add_module(quandelibc src/python_wrapper.cpp ${QLIBC_SOURCES})

target_compile_definitions(quandelibc PRIVATE VERSION_INFO=${VERSION_INFO})

add_executable(test_permanent-int src/test_permanent.cpp ${QLIBC_SOURCES})
target_link_libraries(test_permanent-int ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(test_permanent-int PUBLIC P_INT)

add_executable(test_permanent-float src/test_permanent.cpp ${QLIBC_SOURCES})
target_link_libraries(test_permanent-float ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(test_permanent-float PUBLIC P_FLOAT)

add_executable(test_permanent-complex src/test_permanent.cpp ${QLIBC_SOURCES})
target_link_libraries(test_permanent-complex ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(test_permanent-complex PUBLIC P_COMPLEX)

## ----------------------- Tests ----------------------- ##
include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()
