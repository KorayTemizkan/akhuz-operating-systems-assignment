#include "scheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

// Kuyruklar
QueueHandle_t xQueueRealTime;
QueueHandle_t xQueueHigh;
QueueHandle_t xQueueMedium;
QueueHandle_t xQueueLow;

extern TaskHandle* taskList;
extern int taskCount;
int globalTime = 0;

// Renk kodları
const char* get_color(int id) {
    static const char* colors[] = {"\033[0;31m", "\033[0;32m", "\033[0;33m", "\033[0;34m", "\033[0;35m", "\033[0;36m"};
    return colors[id % 6];
}
#define RESET_COLOR "\033[0m"

// İşçi Task (Dummy)
void GeneralTaskFunction(void* pvParameters) {
    TaskHandle* t = (TaskHandle*) pvParameters;

    print_status("başladı", t);

    for (;;) {
        if (t->remainingTime <= 0) {
            print_status("sonlandi", t);
            vTaskDelete(NULL);
        }

        print_status("yürütülüyor", t);

        /* 1 saniye bekle — tick hızınız 1000 ise pdMS_TO_TICKS(1000) = 1s */
        vTaskDelay(pdMS_TO_TICKS(1000));

        t->remainingTime--;
    }
}

// Kuyruğa Ekle
void add_to_queue(TaskHandle* task) {
    switch(task->priority) {
        case 0: xQueueSend(xQueueRealTime, &task, 0); break;
        case 1: xQueueSend(xQueueHigh,     &task, 0); break;
        case 2: xQueueSend(xQueueMedium,   &task, 0); break;
        case 3: xQueueSend(xQueueLow,      &task, 0); break;
    }
}

// Yazdırma (Flush eklenmiş)
void print_status(const char* action, TaskHandle* t)
{
    TickType_t ticks = xTaskGetTickCount();
    double secs = (double)ticks / (double)configTICK_RATE_HZ;
    printf("%0.4f sn %s %s \t(id:%04d \töncelik:%d \tkalan süre:%d sn)\n",
           secs, t->taskName, action, t->id, t->priority, t->remainingTime);
    fflush(stdout);
}

// --- ANA KONTROL TASKI ---
void SchedulerController(void* pvParameters) {
    (void) pvParameters;
    TaskHandle* currentTask = NULL;

    // Başlangıç Debug Mesajı
    printf("\n[DEBUG] Scheduler Controller Basladi. Zaman: %d\n", globalTime);
    fflush(stdout);

    while(1) {
        // 1. Yeni Gelenleri Kontrol Et
        for(int i=0; i < taskCount; i++) {
            if(taskList[i].arrivalTime == globalTime) {
                taskList[i].state = TASK_READY;
                add_to_queue(&taskList[i]);
                // Debug için isteğe bağlı
                // printf("[DEBUG] Task kuyruga eklendi: %s\n", taskList[i].taskName);
                // fflush(stdout);
            }
        }

        // 2. Kuyruklardan Görev Seç
        QueueHandle_t activeQueue = NULL;
        if(uxQueueMessagesWaiting(xQueueRealTime) > 0) {
            xQueueReceive(xQueueRealTime, &currentTask, 0);
        } 
        else if(uxQueueMessagesWaiting(xQueueHigh) > 0) {
            xQueueReceive(xQueueHigh, &currentTask, 0);
        }
        else if(uxQueueMessagesWaiting(xQueueMedium) > 0) {
            xQueueReceive(xQueueMedium, &currentTask, 0);
        }
        else if(uxQueueMessagesWaiting(xQueueLow) > 0) {
            xQueueReceive(xQueueLow, &currentTask, 0);
        }
        else {
            currentTask = NULL;
        }

        // 3. Yürütme veya Bekleme
        if(currentTask != NULL) {
            if(currentTask->burstTime == currentTask->remainingTime)
                print_status("başladı", currentTask);
            else
                print_status("yürütülüyor", currentTask);

            // Simülasyon zamanını ilerlet
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            currentTask->remainingTime--;
            globalTime++;

            // Task Bitti mi?
            if(currentTask->remainingTime <= 0) {
                currentTask->state = TASK_TERMINATED;
                print_status("sonlandı", currentTask);
            } 
            else {
                // Feedback Mantığı
                if(currentTask->priority == 0) {
                    add_to_queue(currentTask); // Priority 0 değişmez
                }
                else if(currentTask->priority < 3) {
                    currentTask->priority++; // Öncelik düşür
                    add_to_queue(currentTask);
                }
                else {
                    add_to_queue(currentTask); // Zaten en düşükte
                }
            }
        } 
        else {
            // IDLE Durumu (Boşta)
            // Eğer hiç task yoksa burası çalışır. Ekrana basmıyorsa sistem kilitlenmiş sanabilirsin.
            // Debug için bunu açabilirsin:
            // printf("[IDLE] %d.0000 sn Sistem Bosta\n", globalTime);
            // fflush(stdout);
            
            vTaskDelay(pdMS_TO_TICKS(1000));
            globalTime++;
        }

        // 100 saniye sınırı
        if(globalTime > 100) {
            printf("Simulasyon Suresi Doldu.\n");
            fflush(stdout);
            vTaskEndScheduler();
        }
    }
}

void init_scheduler() {
    xQueueRealTime = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueHigh     = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueMedium   = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueLow      = xQueueCreate(50, sizeof(TaskHandle*));

    xTaskCreate(SchedulerController, "Controller", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 5, NULL);
}

/* DİKKAT: create_tasks_from_list fonksiyonu artık src/tasks.c'de tanımlı.
   Buradaki (önceki) tanımı kaldırdık; eğer scheduler tarafında benzer bir yardımcı
   fonksiyon gerekiyorsa farklı bir isimle ekleyin (ör. scheduler_create_tasks). */