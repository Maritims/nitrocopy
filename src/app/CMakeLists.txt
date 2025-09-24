add_executable(NitroCopy main.c)

target_link_libraries(NitroCopy PRIVATE core m)

if(WIN32)
    target_link_libraries(NitroCopy PRIVATE nitro_compat_win9x)
    target_link_options(NitroCopy PRIVATE -static -static-libgcc -static-libstdc++)
elseif(UNIX)
    target_link_libraries(NitroCopy PRIVATE nitro_compat_linux)
endif()
