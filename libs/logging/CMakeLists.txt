set(LIB_NAME logging)

add_library(${LIB_NAME} STATIC
    src/log.c
)

target_include_directories(${LIB_NAME} PUBLIC
    include
)



