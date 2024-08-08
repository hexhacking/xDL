cmake_minimum_required(VERSION 3.22.1)
project(sample)

if(DEPENDENCY_ON_LOCAL_LIBRARY)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../xdl/local_dependency.cmake)
else()
    find_package(xdl REQUIRED CONFIG)
endif()

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_17)
target_compile_options(sample PUBLIC -std=c17 -Weverything -Werror)
target_link_libraries(sample xdl::xdl log)

if((${ANDROID_ABI} STREQUAL "arm64-v8a") OR (${ANDROID_ABI} STREQUAL "x86_64"))
    target_link_options(sample PUBLIC "-Wl,-z,max-page-size=16384")
endif()
