cmake_minimum_required(VERSION 3.4...3.18)
project(quandelibc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build unit tests" OFF)

include(CheckCSourceCompiles)

check_c_source_compiles("
      #include <immintrin.h>
      void floats_add(float *dest, float *a, float *b, unsigned size) {
        for (; size >= 8; size -= 8, dest += 8, a += 8, b += 8) {
          _mm256_storeu_ps(dest, _mm256_add_ps(_mm256_loadu_ps(a), _mm256_loadu_ps(b)));
        }
      }
      int main(int argc, char **argv) {
        floats_add((float*)0, (float*)0, (float*)0, 0);
        return 0;
      }" COMPILER_SUPPORTS_AVX)

if (COMPILER_SUPPORTS_AVX)
    message("AVX support detected")
    add_compile_options(-mavx)
else ()
    message("AVX support not detected")
endif ()

if (MSVC)
    add_compile_options(/arch:AVX2 /MT /Zc:__cplusplus)
else (MSVC)
    add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-zero-length-array)
endif (MSVC)

find_package(Threads)

set(QLIBC_SOURCES
        src/fockstate.cpp src/fockstate.h
        src/annotation.h src/annotation.cpp
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
if (BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
endif ()
