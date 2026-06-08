cmake_minimum_required(VERSION 3.10)
project(CBUtilLib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_XCODE_ATTRIBUTE_RUN_CLANG_STATIC_ANALYZER "YES")

if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        HINTS "/opt/homebrew/opt/llvm/bin"
    )

    if(CLANG_TIDY_EXE)
        # 1. Base Clang-Tidy call configuration
        set(CLANG_TIDY_ARGS "${CLANG_TIDY_EXE};-checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.deprecatedOrUnsafeBufferHandling")

        # 2. On Apple machines, append the current platform SDK directory path
        if(APPLE)
            execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE MAC_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
            list(APPEND CLANG_TIDY_ARGS "--extra-arg=-isysroot" "--extra-arg=${MAC_SDK_PATH}")
        endif()

        # 3. Apply the modified list to CMake
        set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_ARGS}")
        message(STATUS "Clang-Tidy Static Analyzer enabled automatically.")
    else()
        message(STATUS "clang-tidy binary not found. Skipping analysis pass.")
    endif()
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

target_compile_definitions(CBUtil PRIVATE 
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:ClangAnalysis>,$<CONFIG:ClangOptional>,$<CONFIG:ClangAnalysisOptional>>:DEBUG_OUTPUT>
    $<$<OR:$<CONFIG:ClangOptional>,$<CONFIG:ClangAnalysisOptional>>:USE_OPTIONAL>
)

install(TARGETS CBUtil
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

include(CTest)

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  add_subdirectory(tests)
endif()
