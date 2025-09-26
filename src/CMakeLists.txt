add_subdirectory(platform)

add_library(logging STATIC logging.c)

add_library(copier STATIC
    copy_process.c
    copier.c
)
target_link_libraries(copier PUBLIC
    console
    file
    logging
    m
)

add_executable(NitroCopy main.c)
target_link_libraries(NitroCopy PUBLIC
    copier
)
