cmake_minimum_required(VERSION 3.13)
project(WiFi_Sim C)

# --------------------------------------------------------------------------
# 路徑設定修正
# --------------------------------------------------------------------------
# 原本在 WiFi_Sim 資料夾，所以要往上找三層 (../../../)
# 現在搬到了 Posix_GCC 資料夾，只要往上找兩層 (../../) 就可以找到 Source
set(FREERTOS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../Source)

# --------------------------------------------------------------------------
# Include 路徑
# --------------------------------------------------------------------------
include_directories(
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix
    ${CMAKE_CURRENT_LIST_DIR}    # 修正：FreeRTOSConfig.h 現在就在當前目錄 (.)
)

# --------------------------------------------------------------------------
# 來源檔案設定
# --------------------------------------------------------------------------
set(FREERTOS_KERNEL_SOURCES
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/tasks.c
    # ${FREERTOS_DIR}/list.c  <-- 移除：這行重複了，幫您刪掉
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/portable/MemMang/heap_3.c
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix/port.c
    ${FREERTOS_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

# --------------------------------------------------------------------------
# 產生執行檔
# --------------------------------------------------------------------------
add_executable(WiFi_Sim
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/app_hooks.c
    ${FREERTOS_KERNEL_SOURCES}
)

# macOS 連 pthread
target_link_libraries(WiFi_Sim PRIVATE pthread)

# 定義
target_compile_definitions(WiFi_Sim PRIVATE projCOVERAGE_TEST=0)
target_compile_definitions(WiFi_Sim PRIVATE projENABLE_TRACING=0)
