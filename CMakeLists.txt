cmake_minimum_required(VERSION 3.13)
project(WiFi_Sim C)

set(FREERTOS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../Source)

include_directories(
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix
    ${CMAKE_CURRENT_LIST_DIR} 
)

set(FREERTOS_KERNEL_SOURCES
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/portable/MemMang/heap_3.c
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix/port.c
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

add_executable(WiFi_Sim
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/app_hooks.c
    ${FREERTOS_KERNEL_SOURCES}
)

target_link_libraries(WiFi_Sim PRIVATE pthread)

target_compile_definitions(WiFi_Sim PRIVATE projCOVERAGE_TEST=0)
target_compile_definitions(WiFi_Sim PRIVATE projENABLE_TRACING=0)
