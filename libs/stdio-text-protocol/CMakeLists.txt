set(LIB_NAME stdio_text_protocol)

add_library(${LIB_NAME} STATIC
    src/stdio-text-protocol.c
)

target_include_directories(${LIB_NAME} PUBLIC
    include
)

target_link_libraries(${LIB_NAME}
    PRIVATE pico_stdlib
  
    
)

