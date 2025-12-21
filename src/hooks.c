#include "FreeRTOS.h"
#include "task.h"

/* Boş, hafif hook implementasyonları.
   NOT: vApplicationIdleHook içinde bloklayan çağrı (xQueue... bekleme vb.) yapmayın. */
void vApplicationIdleHook(void)
{
    /* Idle sırasında yapılacak çok hafif işler varsa buraya koyun.
       Burada hiçbir şey yapmıyoruz. */
    (void)0;
}

void vApplicationTickHook(void)
{
    /* Tick başına yapılacak hızlı işlemler varsa buraya koyun.
       Çok kısa olmalı; ISR gibi düşünün. */
    (void)0;
}