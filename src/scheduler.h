#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>

extern TaskHandle_t xSchedulerTaskHandle;

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
    int lastActiveTime;
    TaskState state;        // O anki durumu
    int currentQueue;       // Hangi kuyrukta? (0, 1, 2, 3)
    int colorCode;          // Terminalde hangi renkle yazacak?
    
    // FreeRTOS Handle (İleride void* yerine TaskHandle_t olacak)
    void* taskHandle;       
} TaskHandle;

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// Global Kuyruk Handle'ları (Diğer dosyalardan erişilebilsin diye extern yapıyoruz)
extern QueueHandle_t xQueueRealTime; // Seviye 0
extern QueueHandle_t xQueueHigh;     // Seviye 1
extern QueueHandle_t xQueueMedium;   // Seviye 2
extern QueueHandle_t xQueueLow;      // Seviye 3
extern TaskHandle* taskList;
extern int taskCount;
// Global Fonksiyon Prototipleri
void init_scheduler();
void read_input_file(const char* filename);
void print_task_info(TaskHandle* task);

void GeneralTaskFunction(void* pvParameters);
void add_to_queue(TaskHandle* task);
void print_status(const char* action, TaskHandle* t);
void create_tasks_from_list(TaskHandle* list, int count);

#endif