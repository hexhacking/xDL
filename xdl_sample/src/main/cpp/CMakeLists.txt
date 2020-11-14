cmake_minimum_required(VERSION 3.4.1)

add_compile_options(
        -std=c11
        -Weverything
        -Werror)

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_DIRNAME)
link_directories(
        ../../../../xdl_lib/build/intermediates/stripped_native_libs/${BUILD_TYPE_DIRNAME}/out/lib/${ANDROID_ABI}
)

add_library(sample SHARED
        sample.c)

target_include_directories(sample PUBLIC
        ../../../../xdl_lib/src/main/cpp)

target_link_libraries(sample
        xdl
        log)
