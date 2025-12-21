#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>

// Task Durumları
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED,
    TASK_TERMINATED
} TaskState;

// Task Control Block (TCB) - Her bir görevi temsil eden yapı
typedef struct {
    int id;                 // Görev ID'si (Otomatik atanabilir veya satır no)
    char taskName[16];      // Örn: "T1"
    int arrivalTime;        // Varış zamanı (ms veya sn)
    int priority;           // 0 (RealTime), 1 (High), 2 (Medium), 3 (Low)
    int burstTime;          // Toplam çalışma süresi (Gereken süre)
    int remainingTime;      // Kalan süre (Başlangıçta burstTime'a eşit)
    
    TaskState state;        // O anki durumu
    int currentQueue;       // Hangi kuyrukta? (0, 1, 2, 3)
    int colorCode;          // Terminalde hangi renkle yazacak?
    
    // FreeRTOS Handle (İleride void* yerine TaskHandle_t olacak)
    void* taskHandle;       
} TaskHandle;

// Global Fonksiyon Prototipleri
void init_scheduler();
void read_input_file(const char* filename);
void print_task_info(TaskHandle* task);

#endif