# Separate programs keep each test group within the Z80 address space.
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
find_program(UCSIM_Z80_EXECUTABLE NAMES ucsim_z80)
if(NOT UCSIM_Z80_EXECUTABLE)
    message(FATAL_ERROR "SDCC tests require ucsim_z80 on PATH")
endif()

function(add_sdcc_test name source entry_point)
    add_executable(${name} runtime.c "../${source}Test.c")
    set_source_files_properties(runtime.c "../${source}Test.c" PROPERTIES
        OBJECT_DEPENDS "${PROJECT_SOURCE_DIR}/tests/sdcc/compat.h")
    target_compile_definitions(${name} PRIVATE TEST_FUNCTION=${entry_point})
    target_link_libraries(${name} PRIVATE CBUtil)
    target_link_options(${name} PRIVATE --data-loc 0xf000)
    add_test(NAME ${name}
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/run.py"
            "${UCSIM_Z80_EXECUTABLE}" "$<TARGET_FILE:${name}>")
    set_tests_properties(${name} PROPERTIES TIMEOUT 65)
endfunction()

add_sdcc_test(List List LinkedList_tests)
add_sdcc_test(IntDict IntDic intdict_tests)
add_sdcc_test(StrDict StrDic strdict_tests)
add_sdcc_test(PtrDict PtrDic ptrdict_tests)
add_sdcc_test(StrBuf StrBuf StringBuffer_tests)
add_sdcc_test(StrExt StrExt StrExtra_tests)
add_sdcc_test(Macro Macro MacroUtils_tests)
