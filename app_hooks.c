#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )

static StaticTask_t xIdleTCB;
static StackType_t  uxIdleStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t  **ppxIdleTaskStackBuffer,
                                    uint32_t     *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTCB;
    *ppxIdleTaskStackBuffer = uxIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTCB;
static StackType_t  uxTimerStack[ configTIMER_TASK_STACK_DEPTH ];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t  **ppxTimerTaskStackBuffer,
                                     uint32_t     *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTCB;
    *ppxTimerTaskStackBuffer = uxTimerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif /* configSUPPORT_STATIC_ALLOCATION */

void vApplicationIdleHook( void ) { /* 沒事做時 CPU 休息 */ }
void vApplicationTickHook( void ) { /* 每個 tick 呼叫，避免block太久 */ }
void vApplicationDaemonTaskStartupHook( void ) { /* Timer/daemon task 啟動後一次 */ }

void vApplicationMallocFailedHook( void )
{
    /* 動態配置失敗 */
    printf("Malloc failed!\n");
    abort();
}

void vAssertCalled( const char *file, unsigned long line )
{
    printf("ASSERT: %s:%lu\n", file, line);
    fflush(stdout);
    abort();
}
