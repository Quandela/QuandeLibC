cmake_minimum_required(VERSION 3.12)

project(quandelibc LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.1
)
FetchContent_MakeAvailable(Catch2)

set(QLIBC_NESTED_SOURCES)
foreach(SRC ${QLIBC_SOURCES})
    list(APPEND QLIBC_NESTED_SOURCES ../${SRC})
endforeach()
add_executable(quandelibcTests main_tests.cpp ${QLIBC_NESTED_SOURCES}
        test_fockstate.cpp
        test_fs_array.cpp)

target_link_libraries(quandelibcTests PRIVATE Catch2::Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(quandelibcTests)