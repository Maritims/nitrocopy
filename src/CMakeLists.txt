add_subdirectory(platform)

add_library(logging STATIC logging.c)

add_library(word16 STATIC
    word16.c
)

add_library(fletcher32 STATIC
    fletcher32.c
)
target_link_libraries(fletcher32 PUBLIC
    word16
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
    copier
)
