#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "FreeRTOS.h"

// Global değişken: Okunan taskları tutacağımız dizi ve sayısı
TaskHandle* taskList = NULL;
int taskCount = 0;

// Yardımcı Fonksiyon: Dosyadaki satır sayısını bulur
int count_lines(const char* filename) {
      FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    int lines = 0;
    char buf[256];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *p = buf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') continue;
        lines++;
    }
    fclose(fp);
    return lines;
}

void create_tasks_from_list(TaskHandle* list, int count) {
    for (int i = 0; i < count; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "%s", list[i].taskName);
        BaseType_t res = xTaskCreate( GeneralTaskFunction,
                                      name,
                                      configMINIMAL_STACK_SIZE,
                                      &list[i],
                                      /* maplama: FreeRTOS priority >=1 */ (list[i].priority + 1),
                                      NULL );
        if (res != pdPASS) {
            printf("HATA: Gorev olusturulamadi: %s\n", name);
        }
    }
}

// ANA FONKSİYON: Dosyayı okur ve struct dizisini doldurur
void read_input_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Dosya acilamadi");
        exit(EXIT_FAILURE);
    }

    // Önce kaç task var onu bulalım (Basit yöntem: satır sayısını saymayıp dinamik büyütebilirdik ama bu daha güvenli)
    // Şimdilik 100 task limitimiz olsun veya dinamik realloc yapalım.
    // Basitlik adına sabit boyutlu bir buffer ile okuyalım.
    
    taskList = (TaskHandle*)malloc(sizeof(TaskHandle) * 100); // Max 100 task
    char line[128];
    int id_counter = 1;

    printf("--- Dosya Okuma Basladi: %s ---\n", filename);

    while (fgets(line, sizeof(line), fp)) {
        // Boş satırları atla
        if (strlen(line) < 3) continue;

        int arrival, priority, burst;
        // Format: Varış, Öncelik, Süre (Örn: 12, 0, 1)
        if (sscanf(line, "%d, %d, %d", &arrival, &priority, &burst) == 3) {
            
            taskList[taskCount].id = id_counter++;
            sprintf(taskList[taskCount].taskName, "T%d", taskList[taskCount].id);
            taskList[taskCount].arrivalTime = arrival;
            taskList[taskCount].priority = priority;
            taskList[taskCount].burstTime = burst;
            taskList[taskCount].remainingTime = burst;
            taskList[taskCount].state = TASK_READY;
            taskList[taskCount].currentQueue = priority; // Başlangıçta önceliğine göre kuyruğa girer
            
            // Renk atama (Rastgele basit bir mantık)
            // ANSI Renk kodları: 31=Kırmızı, 32=Yeşil, 33=Sarı, 34=Mavi, 35=Mor, 36=Turkuaz
            taskList[taskCount].colorCode = 31 + (taskCount % 6);

            printf("Okundu -> Task: %s | Gelis: %d | Oncelik: %d | Sure: %d\n", 
                 taskList[taskCount].taskName, arrival, priority, burst);
            
            taskCount++;
        }
    }

    fclose(fp);
    printf("--- Toplam %d task yüklendi. ---\n", taskCount);
}