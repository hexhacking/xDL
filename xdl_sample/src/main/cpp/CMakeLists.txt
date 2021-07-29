cmake_minimum_required(VERSION 3.18.1)
project(sample)

if(DEPENDENCY_ON_LOCAL_LIBRARY)
    # import xdl from local-project
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_DIRNAME)
    if(${BUILD_TYPE_DIRNAME} STREQUAL "relwithdebinfo")
        set(BUILD_TYPE_DIRNAME "release")
    endif()
    set(XDL_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../xdl)
    add_library(xdl::xdl SHARED IMPORTED)
    set_target_properties(xdl::xdl PROPERTIES
            IMPORTED_LOCATION ${XDL_BASE}/build/intermediates/stripped_native_libs/${BUILD_TYPE_DIRNAME}/out/lib/${ANDROID_ABI}/libxdl.so
            INTERFACE_INCLUDE_DIRECTORIES ${XDL_BASE}/src/main/cpp/include)
else()
    # import xdl from maven
    find_package(xdl REQUIRED CONFIG)
endif()

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_11)
target_compile_options(sample PUBLIC -Weverything -Werror)
target_link_libraries(sample xdl::xdl log)
