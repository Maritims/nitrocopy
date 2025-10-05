add_library(fletcher32 STATIC
    fletcher32.c
)

target_include_directories(fletcher32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fletcher32 PUBLIC
    word16
)

add_executable(test_fletcher32 test_fletcher32.c)
target_link_libraries(test_fletcher32 fletcher32)
add_test(NAME test_fletcher32 COMMAND test_fletcher32)

