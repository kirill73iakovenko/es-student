set(LIB_NAME device)

add_library(${LIB_NAME} STATIC
    src/device.c
)

target_include_directories(${LIB_NAME} PUBLIC
    include
)

target_link_libraries(${LIB_NAME}
    PRIVATE pico_stdlib
    PUBLIC pico_unique_id
)

target_compile_definitions(${LIB_NAME} PRIVATE
    DEVICE_BOARD="${PICO_BOARD}"
)