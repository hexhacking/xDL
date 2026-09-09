cmake_minimum_required(VERSION 4.0.2)
project(sample)

if(DEPENDENCY_ON_LOCAL_LIBRARY)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../xdl/local_dependency.cmake)
else()
    find_package(xdl REQUIRED CONFIG)
endif()

set(TARGET "sample")
add_library(${TARGET} SHARED sample.c)
target_compile_features(${TARGET} PRIVATE c_std_11)
target_compile_options(${TARGET} PRIVATE -Weverything -Werror)
target_link_libraries(${TARGET} xdl::xdl log)

if((${ANDROID_ABI} STREQUAL "arm64-v8a") OR (${ANDROID_ABI} STREQUAL "x86_64"))
    target_link_options(${TARGET} PRIVATE "-Wl,-z,max-page-size=16384")
endif()
