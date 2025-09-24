add_library(nitro_compat_win9x STATIC
    nitro_console.c
    nitro_file.c
)

target_include_directories(nitro_compat_win9x
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
