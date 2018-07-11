cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(ccr C)

option(BUILD_TESTS "Build tests" ON)

include(CheckSymbolExists)
include(FindThreads)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

set(CCR_SRC ccr.c ccr.h)

include_directories(.)
add_library(ccr ${CCR_SRC})
set_property(TARGET ccr PROPERTY C_STANDARD 99)

target_link_libraries(ccr Threads::Threads)
if(${CMAKE_C_COMPILER_ID} MATCHES Clang|GNU)
    target_compile_options(ccr PRIVATE -O3 -Wall -Wextra -pedantic)
elseif(${CMAKE_C_COMPILER_ID} MATCHES MSVC)
    target_compile_options(ccr PRIVATE /W4)
endif()

if(BUILD_TESTS)
    #List of test programs that use libccr
    set(CCR_TESTS test_lib test_macro)
    foreach(test_src ${CCR_TESTS})
        add_executable(${test_src} ${test_src}.c)
        target_link_libraries(${test_src} Threads::Threads ccr)
        set_property(TARGET ${test_src} PROPERTY C_STANDARD 99)
    endforeach()
endif()

install(TARGETS ccr DESTINATION lib)
install(FILES ccr.h DESTINATION include)
