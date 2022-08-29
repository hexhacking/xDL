cmake_minimum_required(VERSION 3.22.1)
project(sample)

find_package(xdl REQUIRED CONFIG)

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_17)
target_compile_options(sample PUBLIC -std=c17 -Weverything -Werror)
target_link_libraries(sample xdl::xdl log)
