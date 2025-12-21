#include "scheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

extern TaskHandle* taskList;
extern int taskCount;
int globalTime = 0;
TaskHandle_t xSchedulerTaskHandle = NULL; // Controller Handle

// Kuyruklar (Queue) - TaskHandle pointer tutacaklar
QueueHandle_t xQueueRealTime; // Prio 0
QueueHandle_t xQueueHigh;     // Prio 1
QueueHandle_t xQueueMedium;   // Prio 2
QueueHandle_t xQueueLow;      // Prio 3

// Yardımcı: Renkli yazdırma
const char* get_color(int id) {
    // İsteğe bağlı renk kodları
    return ""; 
}

// --- İŞÇİ TASK (WORKER) ---
// Bu task sadece Scheduler izin verdiğinde 1 sn (simüle) çalışır.
void GeneralTaskFunction(void* pvParameters) {
    TaskHandle* t = (TaskHandle*) pvParameters;

    while(1) {
        // 1. Scheduler beni uyandırana kadar bekle
        // (xTaskCreate sonrası zaten Suspend edildi, Resume edilince buraya düşer)
        
        // 2. İşlem yap (Kalan süreyi azalt)
        if (t->remainingTime > 0) {
            t->remainingTime--;
        }

        // 3. İşim bitti, Yöneticiye haber ver (Benim 1 saniyem doldu)
        xTaskNotifyGive(xSchedulerTaskHandle);

        // 4. Tekrar uyku moduna geç (Yönetici bir daha çağırana kadar)
        vTaskSuspend(NULL);
    }
}

// --- YARDIMCI: Kuyruğa Ekle ---
void add_to_queue(TaskHandle* task) {
    switch(task->priority) {
        case 0: xQueueSend(xQueueRealTime, &task, 0); break;
        case 1: xQueueSend(xQueueHigh,     &task, 0); break;
        case 2: xQueueSend(xQueueMedium,   &task, 0); break;
        default: xQueueSend(xQueueLow,     &task, 0); break;
    }
}

// --- YÖNETİCİ (CONTROLLER) TASK ---
void SchedulerController(void* pvParameters) {
    xSchedulerTaskHandle = xTaskGetCurrentTaskHandle();
    printf("Simulasyon Basliyor...\n");

    while(1) {
        // --- 1. ZAMANAŞIMI KONTROLÜ (Starvation: 20 sn kuralı) ---
        for(int i=0; i < taskCount; i++) {
            if(taskList[i].state != TASK_TERMINATED && taskList[i].remainingTime > 0) {
                // Eğer (Şu anki zaman - Son Aktif Zaman) >= 20 ise ÖLDÜR
                if((globalTime - taskList[i].lastActiveTime) >= 20) {
                    taskList[i].state = TASK_TERMINATED;
                    printf("%d.0000 sn proses zamanasımı\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n", 
                           globalTime, taskList[i].id, taskList[i].priority, taskList[i].remainingTime);
                }
            }
        }

        // --- 2. YENİ GELENLERİ KONTROL ET ---
        for(int i=0; i < taskCount; i++) {
            if(taskList[i].arrivalTime == globalTime) {
                taskList[i].state = TASK_READY;
                add_to_queue(&taskList[i]);
            }
        }

        // --- 3. KUYRUKLARDAN GÖREV SEÇ ---
        TaskHandle* currentTask = NULL;
        
        // Prio 0 (RealTime) en öncelikli
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

        // --- 4. GÖREV VARSA ÇALIŞTIR ---
        if(currentTask != NULL) {
            // Eğer task zaman aşımına uğramışsa (kuyruktan çektik ama ölü) atla
            if(currentTask->state == TASK_TERMINATED) {
                continue;
            }

            // Çıktı formatı (Türkçe karakterler)
            if(currentTask->burstTime == currentTask->remainingTime) {
                 printf("%d.0000 sn proses başladı\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n", 
                        globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);
            } else {
                 printf("%d.0000 sn proses yürütülüyor\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n", 
                        globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);
            }

            // Taskı Çalıştır
            vTaskResume(currentTask->taskHandle);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // Task çalıştı, zamanı güncelle
            globalTime++; 
            
            // Task çalıştığı için "Son Aktif Zaman" güncellenir (Zamanaşımı sıfırlanır)
            currentTask->lastActiveTime = globalTime;

            if(currentTask->remainingTime <= 0) {
                currentTask->state = TASK_TERMINATED;
                printf("%d.0000 sn proses sonlandı\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n", 
                       globalTime, currentTask->id, currentTask->priority, 0);
            } 
            else {
                // FEEDBACK MEKANİZMASI
                if(currentTask->priority == 0) {
                    add_to_queue(currentTask); // Değişmez
                } 
                else {
                    // Öncelik değerini artır (Önceliği düşür). Sınır koymuyoruz (Hoca 5'e kadar çıkmış).
                    currentTask->priority++;
                    
                    printf("%d.0000 sn proses askıda\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n", 
                           globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);
                    
                    add_to_queue(currentTask);
                }
            }
        } 
        else {
            // IDLE Durumu
            globalTime++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // PC fanı için küçük bekleme

        if(globalTime > 60) { // Hocanın çıktısı 44 sn sürüyor, 60 yeterli
            printf("Simulasyon Bitti.\n");
            vTaskEndScheduler();
            break;
        }
    }
}

void init_scheduler() {
    // Kuyrukları task pointerı tutacak şekilde oluştur
    xQueueRealTime = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueHigh     = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueMedium   = xQueueCreate(50, sizeof(TaskHandle*));
    xQueueLow      = xQueueCreate(50, sizeof(TaskHandle*));

    // Controller en yüksek önceliğe sahip olmalı ki her şeye o karar versin
    xTaskCreate(SchedulerController, "Controller", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES - 1, NULL);
}