add_library(nitro_compat_linux STATIC
    nitro_console.c
    nitro_file.c
)

target_include_directories(nitro_compat_linux
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
