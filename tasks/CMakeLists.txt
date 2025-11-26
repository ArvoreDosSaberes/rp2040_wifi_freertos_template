
file(GLOB RACK_TASKS_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

add_library(tasks STATIC
    ${RACK_TASKS_SOURCES}
)

target_include_directories(tasks PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(tasks
        pico_stdlib
        FreeRTOS-Kernel-Heap4
        pico_cyw43_arch_lwip_threadsafe_background

        log_vt100   
)