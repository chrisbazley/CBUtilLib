file(GLOB SOURCES "*.c")
file(GLOB PRIVATE_HEADERS "*.h")

add_executable(Tests ${SOURCES})
target_link_libraries(Tests PRIVATE CBUtil)
add_test(NAME "tests" COMMAND Tests)
