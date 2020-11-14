cmake_minimum_required(VERSION 3.4.1)

add_compile_options(
        -std=c11
        -Weverything
        -Werror)

file(GLOB XDL_SRC
        *.c)

add_library(xdl SHARED
        ${XDL_SRC})

target_include_directories(xdl PUBLIC
        .)

target_link_libraries(xdl
        dl)

if(USEASAN)

target_compile_options(xdl PUBLIC
        -fsanitize=address
        -fno-omit-frame-pointer)

set_target_properties(xdl PROPERTIES
        LINK_FLAGS " \
        -fsanitize=address")

else()

target_compile_options(xdl PUBLIC
        -Oz
        -flto
        -ffunction-sections
        -fdata-sections)

set_target_properties(xdl PROPERTIES
        LINK_FLAGS " \
        -O3 \
        -flto \
        -Wl,--exclude-libs,ALL \
        -Wl,--gc-sections \
        -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/xdl.map.txt")

endif()
