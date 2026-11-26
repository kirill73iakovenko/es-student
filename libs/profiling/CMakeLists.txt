set(LIB_NAME profiling)

add_library(${LIB_NAME} STATIC
    src/profiling.c
)

target_include_directories(${LIB_NAME} PUBLIC
    include
)

target_link_libraries(${LIB_NAME}
    PRIVATE pico_stdlib
    
)

target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

