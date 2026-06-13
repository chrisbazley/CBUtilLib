file(GLOB SOURCES "*.c")
file(GLOB PRIVATE_HEADERS "*.h")

add_executable(CBUtilTests ${SOURCES} ${PRIVATE_HEADERS})
target_link_libraries(CBUtilTests PRIVATE CBUtil)
add_test(NAME "CBUtilTests" COMMAND CBUtilTests)
