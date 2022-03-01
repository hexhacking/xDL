cmake_minimum_required(VERSION 3.18.1)
project(sample)

if(DEPENDENCY_ON_LOCAL_LIBRARY)
    set(XDL_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../xdl)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(XDL_LIB_BASE ${XDL_BASE}/build/intermediates/merged_native_libs/debug)
    else()
        set(XDL_LIB_BASE ${XDL_BASE}/build/intermediates/stripped_native_libs/release)
    endif()
    add_library(xdl::xdl SHARED IMPORTED)
    set_target_properties(xdl::xdl PROPERTIES
            IMPORTED_LOCATION ${XDL_LIB_BASE}/out/lib/${ANDROID_ABI}/libxdl.so
            INTERFACE_INCLUDE_DIRECTORIES ${XDL_BASE}/src/main/cpp/include)
else()
    find_package(xdl REQUIRED CONFIG)
endif()

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_11)
target_compile_options(sample PUBLIC -Weverything -Werror)
target_link_libraries(sample xdl::xdl log)
