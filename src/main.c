#include <stdio.h>
#include "scheduler.h"
#include "FreeRTOS.h"
#include "task.h"

/* tasks.c içinde tanımlı global değişkenler / fonksiyonlar */
extern TaskHandle* taskList;
extern int taskCount;
extern void read_input_file(const char* filename);
extern void create_tasks_from_list(TaskHandle* list, int count);

int main(int argc, char* argv[]) {
    /* stdout'u anında görmek için unbuffered yap */
    setbuf(stdout, NULL);

    if (argc < 2) {
        printf("Kullanim: %s <giris_dosyasi>\n", argv[0]);
        return 1;
    }

    /* 1) Dosyayı oku ve taskList / taskCount doldurulsun */
    read_input_file(argv[1]);

    /* 2) Okunan listeden FreeRTOS görevlerini oluştur */
    create_tasks_from_list(taskList, taskCount);

    printf("\n[SISTEM] FreeRTOS Kernel Baslatiliyor... (Simulasyon Basladi)\n");
    fflush(stdout);
    printf("[DIAGNOSTIC] vTaskStartScheduler çağrılacak...\n");
    fflush(stdout);

    /* 3) Scheduler'ı başlat (tek çağrı) */
    vTaskStartScheduler();

    /* Scheduler dönmüşse hata var */
    printf("[DIAGNOSTIC] vTaskStartScheduler dondu! (Scheduler baslatilamadi)\n");
    return 1;
}