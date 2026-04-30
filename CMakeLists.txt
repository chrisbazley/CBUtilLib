cmake_minimum_required(VERSION 3.10)
project(CBUtilLib C)

set(CMAKE_C_STANDARD 99)

if(MSVC)
    # MSVC equivalent of -Wall -Wextra is /W4
    add_compile_options(/W4)
else()
    # For GCC and Clang
    add_compile_options(-Wall -Wextra -Wsign-conversion -pedantic)
endif()

file(GLOB SOURCES "*.c")
file(GLOB PUBLIC_HEADERS "*.h")

add_library(CBUtil STATIC ${SOURCES} ${PUBLIC_HEADERS})

set_target_properties(CBUtil PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
    DEBUG_POSTFIX "dbg"
)

target_include_directories(CBUtil PUBLIC .)

target_compile_definitions(CBUtil PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:-O3>
)

install(TARGETS CBUtil
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
