set(LIB_NAME systime)

add_library(${LIB_NAME} STATIC
    src/systime.c
)

target_include_directories(${LIB_NAME} PUBLIC
    include
)

target_link_libraries(${LIB_NAME}
    PRIVATE pico_time
    
)

