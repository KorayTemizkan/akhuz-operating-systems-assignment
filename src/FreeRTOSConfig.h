/* src/FreeRTOSConfig.h */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* PC Simülasyonu için gerekli kütüphaneler */
#include <assert.h>

/* Temel Ayarlar */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     1
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 100000000 ) /* PC'de sanal hız */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )         /* 1000 Hz = 1ms Tick */
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 65 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_RECURSIVE_MUTEXES             1
#define configQUEUE_REGISTRY_SIZE               20
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0
#define configUSE_QUEUE_SETS                    1
#define configUSE_TASK_NOTIFICATIONS            1

/* Software Timer Ayarları */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                20
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* Print fonksiyonları için map */
extern void vPrintString( const char *pcString );
#define configPRINT_STRING( X ) vPrintString( X )

/* Assert Tanımı (Hata ayıklama için çok önemli) */
#define configASSERT( x )     if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* PC Simülasyonu (Posix Port) için özel tanımlar */
#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 0

#endif /* FREERTOS_CONFIG_H */