#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1

#define configCPU_CLOCK_HZ                     8000000UL
#define configTICK_RATE_HZ                     1000U
#define configMAX_PRIORITIES                   3
#define configMINIMAL_STACK_SIZE               128U
#define configTOTAL_HEAP_SIZE                  (6U * 1024U)
#define configMAX_TASK_NAME_LEN                16
#define configUSE_16_BIT_TICKS                 0
#define configIDLE_SHOULD_YIELD                1

#define configSUPPORT_DYNAMIC_ALLOCATION       1
#define configSUPPORT_STATIC_ALLOCATION        0

#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configUSE_MALLOC_FAILED_HOOK           0
#define configCHECK_FOR_STACK_OVERFLOW         0

#define configUSE_MUTEXES                      0
#define configUSE_RECURSIVE_MUTEXES            0
#define configUSE_COUNTING_SEMAPHORES          0
#define configUSE_TIMERS                       0
#define configUSE_CO_ROUTINES                  0

#define configPRIO_BITS                        4
#define configKERNEL_INTERRUPT_PRIORITY        (15U << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY   (5U << 4)

#define INCLUDE_xTaskDelayUntil                1

/* Ghép handler FreeRTOS với tên trong startup.c. */
#define vPortSVCHandler                        SVC_Handler
#define xPortPendSVHandler                     PendSV_Handler
#define xPortSysTickHandler                    SysTick_Handler

#endif