add_library(getopt STATIC
    getopt.c
)

target_include_directories(getopt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(test_swap_with_tail test_swap_with_tail.c)
target_link_libraries(test_swap_with_tail PRIVATE getopt)
add_test(NAME test_swap_with_tail COMMAND test_swap_with_tail)

add_executable(test_getopt test_getopt.c)
target_link_libraries(test_getopt PRIVATE getopt)
add_test(NAME test_getopt COMMAND test_getopt)
