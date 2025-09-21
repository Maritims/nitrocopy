function(add_c_test name)
    add_executable(${name} ${name}.c)
    target_link_libraries(${name} PRIVATE nitro)

    if(UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Cross build: use wine to run Win32 tests
        add_test(NAME ${name} COMMAND wine $<TARGET_FILE:${name}>)
    else()
        # Native build: run directly
        add_test(NAME ${name} COMMAND ${name})
    endif()
endfunction()

add_c_test(cursor_test)
