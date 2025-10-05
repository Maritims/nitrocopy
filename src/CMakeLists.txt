add_subdirectory(fletcher32)
add_subdirectory(getopt)
add_subdirectory(platform)

add_library(logging STATIC logging.c)

add_library(word16 STATIC
    word16.c
)

add_library(copier STATIC
    copier.c
)
target_link_libraries(copier PUBLIC
    fletcher32
    console
    file
    logging
    m
)

add_executable(NitroCopy main.c)
target_link_libraries(NitroCopy PUBLIC
    getopt
    copier
)
