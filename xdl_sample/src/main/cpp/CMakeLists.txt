cmake_minimum_required(VERSION 3.4.1)

add_compile_options(
        -std=c11
        -Weverything
        -Werror)

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_DIRNAME)
set(XDL_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../xdl_lib)
add_library(xdl SHARED IMPORTED)
set_target_properties(xdl PROPERTIES
        IMPORTED_LOCATION ${XDL_BASE}/build/intermediates/stripped_native_libs/${BUILD_TYPE_DIRNAME}/out/lib/${ANDROID_ABI}/libxdl.so
        INTERFACE_INCLUDE_DIRECTORIES ${XDL_BASE}/src/main/cpp
        )

#set(XDL_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/nativedeps/xdl)
#add_library(xdl SHARED IMPORTED)
#set_target_properties(xdl PROPERTIES
#        IMPORTED_LOCATION ${XDL_BASE}/aar/jni/${ANDROID_ABI}/libxdl.so
#        INTERFACE_INCLUDE_DIRECTORIES ${XDL_BASE}/header
#        )

add_library(sample SHARED
        sample.c)

target_link_libraries(sample
        xdl
        log)
