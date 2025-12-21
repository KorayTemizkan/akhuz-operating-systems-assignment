#include <stdio.h>
#include "scheduler.h"

int main(int argc, char* argv[]) {
    // Argüman kontrolü
    if (argc < 2) {
        printf("Kullanim: ./freertos_sim giris.txt\n");
        return 1;
    }

    printf("Simulasyon Baslatiliyor...\n");
    printf("Giris Dosyasi: %s\n", argv[1]);

    // Dosya okuma işlemi burada yapılacak (Task 1'deki arkadaş yapacak)
    // read_input_file(argv[1]);

    // Scheduler başlatma (Task 3'teki arkadaş yapacak)
    // init_scheduler();

    return 0;
}