#include "scheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

extern TaskHandle* taskList;
extern int taskCount;

static int globalTime = 0;

TaskHandle_t xSchedulerTaskHandle = NULL;

QueueHandle_t xQueueRealTime;
QueueHandle_t xQueueHigh;
QueueHandle_t xQueueMedium;
QueueHandle_t xQueueLow;

// FCFS + non-preemptive RT için: şu an koşan RT task (priority 0)
static TaskHandle* runningRTTask = NULL;

// Yardımcı: Renkli yazdırma (opsiyonel)
const char* get_color(int id) {
    (void)id;
    return "";
}

// --- İŞÇİ TASK (WORKER) ---
// Bu task sadece Scheduler izin verdiğinde 1 sn (simüle) çalışır.
void GeneralTaskFunction(void* pvParameters) {
    TaskHandle* t = (TaskHandle*) pvParameters;

    while (1) {
        // 1) 1 saniyelik iş: kalan süreyi azalt
        if (t->remainingTime > 0) {
            t->remainingTime--;
        }

        // 2) Controller'a "1 saniyem bitti" diye haber ver
        xTaskNotifyGive(xSchedulerTaskHandle);

        // 3) Controller tekrar çağırana kadar kendimi askıya al
        vTaskSuspend(NULL);
    }
}

// --- YARDIMCI: Kuyruğa Ekle ---
void add_to_queue(TaskHandle* task) {
    switch (task->priority) {
        case 0: xQueueSend(xQueueRealTime, &task, 0); break;
        case 1: xQueueSend(xQueueHigh,     &task, 0); break;
        case 2: xQueueSend(xQueueMedium,   &task, 0); break;
        default: xQueueSend(xQueueLow,     &task, 0); break;
    }
}

// --- YÖNETİCİ (CONTROLLER) TASK ---
void SchedulerController(void* pvParameters) {
    (void)pvParameters;

    xSchedulerTaskHandle = xTaskGetCurrentTaskHandle();
    printf("Simulasyon Basliyor...\n");

    while (1) {
        // --- 1) ZAMANAŞIMI KONTROLÜ (Starvation: 20 sn kuralı) ---
        for (int i = 0; i < taskCount; i++) {
            if (taskList[i].state != TASK_TERMINATED && taskList[i].remainingTime > 0) {
                if ((globalTime - taskList[i].lastActiveTime) >= 20) {
                    taskList[i].state = TASK_TERMINATED;
                    printf("%d.0000 sn proses zamanasımı\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n",
                           globalTime, taskList[i].id, taskList[i].priority, taskList[i].remainingTime);

                    // Eğer zaman aşımına uğrayan RT ise ve "runningRTTask" ise temizle
                    if (runningRTTask == &taskList[i]) {
                        runningRTTask = NULL;
                    }
                }
            }
        }

        // --- 2) YENİ GELENLERİ KONTROL ET ---
        for (int i = 0; i < taskCount; i++) {
            if (taskList[i].arrivalTime == globalTime) {
                if (taskList[i].state != TASK_TERMINATED) {
                    taskList[i].state = TASK_READY;
                    add_to_queue(&taskList[i]);
                }
            }
        }

        // --- 3) KUYRUKLARDAN GÖREV SEÇ ---
        TaskHandle* currentTask = NULL;

        // ✅ RT (prio 0) için FCFS + non-preemptive:
        // Eğer hâlâ çalışan bir RT task varsa onu devam ettir.
        if (runningRTTask != NULL &&
            runningRTTask->state != TASK_TERMINATED &&
            runningRTTask->remainingTime > 0) {
            currentTask = runningRTTask;
        }
        // Yoksa RT kuyruğundan sıradakini çek
        else if (uxQueueMessagesWaiting(xQueueRealTime) > 0) {
            xQueueReceive(xQueueRealTime, &currentTask, 0);
            runningRTTask = currentTask; // RT artık "koşan" RT oldu
        }
        // RT yoksa: kullanıcı görevleri
        else if (uxQueueMessagesWaiting(xQueueHigh) > 0) {
            xQueueReceive(xQueueHigh, &currentTask, 0);
        }
        else if (uxQueueMessagesWaiting(xQueueMedium) > 0) {
            xQueueReceive(xQueueMedium, &currentTask, 0);
        }
        else if (uxQueueMessagesWaiting(xQueueLow) > 0) {
            xQueueReceive(xQueueLow, &currentTask, 0);
        }

        // --- 4) GÖREV VARSA ÇALIŞTIR ---
        if (currentTask != NULL) {
            // Kuyruktan çektiğimiz task "ölü" çıkarsa atla
            if (currentTask->state == TASK_TERMINATED || currentTask->remainingTime <= 0) {
                // RT ise runningRTTask temizle
                if (currentTask->priority == 0 && runningRTTask == currentTask) {
                    runningRTTask = NULL;
                }
                // Zamanı ilerletmeden döngüye devam (istersen globalTime++ yapabilirsin)
                continue;
            }

            // Başladı mı / yürütülüyor mu?
            if (currentTask->burstTime == currentTask->remainingTime) {
                printf("%d.0000 sn proses başladı\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n",
                       globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);
            } else {
                printf("%d.0000 sn proses yürütülüyor\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n",
                       globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);
            }

            // Worker'ı 1 saniye çalıştır
            vTaskResume(currentTask->taskHandle);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // 1 saniye geçti
            globalTime++;

            // Bu task bu saniye çalıştı → starvation sayacı sıfırlansın
            currentTask->lastActiveTime = globalTime;

            // İş bitti mi?
            if (currentTask->remainingTime <= 0) {
                currentTask->state = TASK_TERMINATED;

                // RT bittiyse runningRTTask temizle
                if (currentTask->priority == 0 && runningRTTask == currentTask) {
                    runningRTTask = NULL;
                }

                printf("%d.0000 sn proses sonlandı\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n",
                       globalTime, currentTask->id, currentTask->priority, 0);
            } else {
                // --- FEEDBACK MEKANİZMASI ---
                if (currentTask->priority == 0) {
                    // ✅ RT: askıya alma yok, öncelik düşmez.
                    // FCFS olduğu için runningRTTask devam edecek; kuyruğa geri koymaya gerek yok.
                    // (İstersen kuyruğa koyup yine de runningRTTask ile override edebilirsin ama gerek yok.)
                } else {
                    // Kullanıcı görevi: 1 saniye çalıştı, bitmediyse askıya al + öncelik düşür
                    currentTask->priority++; // hoca örneğinde 4-5'e kadar çıkabiliyor

                    printf("%d.0000 sn proses askıda\t\t(id:%04d\toncelik:%d\tkalan sure:%d sn)\n",
                           globalTime, currentTask->id, currentTask->priority, currentTask->remainingTime);

                    add_to_queue(currentTask);
                }
            }
        } else {
            // IDLE
            globalTime++;
        }

        // PC fanı için minik bekleme (simülasyon)
        vTaskDelay(pdMS_TO_TICKS(10));

        // Güvenli çıkış
        if (globalTime > 60) {
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

    // Controller en yüksek önceliğe sahip olmalı
    xTaskCreate(SchedulerController,
                "Controller",
                configMINIMAL_STACK_SIZE * 4,
                NULL,
                configMAX_PRIORITIES - 1,
                NULL);
}
