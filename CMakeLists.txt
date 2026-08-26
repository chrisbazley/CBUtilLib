cmake_minimum_required(VERSION 3.14)
project(CBUtilLib C)

option(USE_OPTIONAL "Enable the _Optional qualifier" OFF)
option(USE_FORTIFY "Enable Fortify memory-allocation checking" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

if(USE_OPTIONAL)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR
            "USE_OPTIONAL requires Christopher Bazley's Clang fork")
    endif()

    add_compile_definitions(USE_OPTIONAL)

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_RUN_CLANG_STATIC_ANALYZER "YES")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_ANALYZER_OTHER_FLAGS
            "-analyzer-checker optionality,core -analyzer-max-loop 32")
    endif()
endif()

if(USE_FORTIFY)
    include(FetchContent)

    FetchContent_Declare(Fortify
        GIT_REPOSITORY https://github.com/chrisbazley/Fortify.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(Fortify)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_XCODE_ATTRIBUTE_RUN_CLANG_STATIC_ANALYZER "YES")

if(MSVC)
    # MSVC equivalent of -Wall -Wextra is /W4
    add_compile_options(/W4 /wd4200 /wd4996 /wd4130)
else()
    # For GCC and Clang
    add_compile_options(-Wall -Wextra -Wsign-compare -pedantic)
endif()

if(ENABLE_CLANG_TIDY)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR "ENABLE_CLANG_TIDY requires Clang")
    endif()

    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        HINTS "/opt/homebrew/opt/llvm/bin"
    )

    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR
            "ENABLE_CLANG_TIDY is ON, but clang-tidy was not found")
    endif()

    set(CMAKE_C_CLANG_TIDY
        "${CLANG_TIDY_EXE}"
        "--checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*"
        "--warnings-as-errors=clang-analyzer-optionality.*"
    )

    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
    message(STATUS "_Optional static analysis enabled")
endif()

if(MSVC)
    # MSVC equivalent of -Wall -Wextra is /W4
    add_compile_options(/W4 /wd4200 /wd4996 /wd4130)
else()
    # For GCC and Clang
    add_compile_options(-Wall -Wextra -Wsign-conversion -pedantic)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "clang")
    set(CMAKE_C_FLAGS_CLANGANALYSIS          "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_CLANGOPTIONAL          "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_CLANGANALYSISOPTIONAL  "${CMAKE_C_FLAGS_DEBUG}")

    add_compile_options(
        $<$<OR:$<CONFIG:ClangAnalysis>,$<CONFIG:ClangAnalysisOptional>>:--analyze>
    )

    set(FORK_CHECKERS "-xanalyzer -analyzer-checker=optionality,nullability,core --analyzer-no-default-checks")
    add_compile_options(
        $<$<CONFIG:ClangAnalysisOptional>:${FORK_CHECKERS}>
    )
endif()

set(SOURCES
    ArgUtils.c
    CSV.c
    FileRWInt.c
    IntDict.c
    IntDictVIt.c
    LinkedList.c
    StrDict.c
    StrDictVIt.c
    StrInflate.c
    StrTail.c
    StringBuf2.c
    StringBuf3.c
    StringBuff.c
    TrigTable.c
)

if(NOT MSVC)
    list(APPEND SOURCES
        Strdup.c
        Stricmp.c
        Strnicmp.c
    )
endif()

file(GLOB PUBLIC_HEADERS "*.h")
file(GLOB PRIVATE_HEADERS "Internal/*.h")

add_library(CBUtil STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})

set_target_properties(CBUtil PROPERTIES 
    PUBLIC_HEADER "${PUBLIC_HEADERS}" 
    DEBUG_POSTFIX "dbg"
    MAP_IMPORTED_CONFIG_CLANG                  DEBUG
    MAP_IMPORTED_CONFIG_CLANGANALYSIS          DEBUG
    MAP_IMPORTED_CONFIG_CLANGOPTIONAL          DEBUG
    MAP_IMPORTED_CONFIG_CLANGANALYSISOPTIONAL  DEBUG
)

target_include_directories(CBUtil PUBLIC .)

if(USE_FORTIFY)
    target_link_libraries(CBUtil PRIVATE Fortify)
endif()

target_compile_definitions(CBUtil PRIVATE 
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:ClangAnalysis>,$<CONFIG:ClangOptional>,$<CONFIG:ClangAnalysisOptional>>:DEBUG_OUTPUT>
    $<$<OR:$<CONFIG:ClangOptional>,$<CONFIG:ClangAnalysisOptional>>:USE_OPTIONAL>
)

if(UNIX)
    target_link_libraries(CBUtil PRIVATE m)
endif()

install(TARGETS CBUtil
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

include(CTest)

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)

    # The tests call Fortify's fault-injection interface directly. Fortify is
    # deliberately a private dependency of CBUtil, so link it separately here
    # to provide the tests with its interface and FORTIFY definition.
    if(USE_FORTIFY)
        target_link_libraries(CBUtilTests PRIVATE Fortify)
    endif()
endif()
