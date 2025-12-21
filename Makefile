# Derleyici ve Bayraklar
CC = gcc
CFLAGS = -Wall -Wextra -g -I. -I./src -I./FreeRTOS/include -I./FreeRTOS/portable/ThirdParty/GCC/Posix -I./FreeRTOS/portable/ThirdParty/GCC/Posix/utils -D_CONSOLE_MODE 

# Kaynak Dosyalar (Senin kodların)
# DİKKAT: tasks.c burada özel muamele görecek
SRCS_MAIN = src/main.c src/scheduler.c src/hooks.c


# FreeRTOS Kaynak Dosyaları
FREERTOS_SRC = FreeRTOS/source/tasks.c \
               FreeRTOS/source/queue.c \
               FreeRTOS/source/list.c \
               FreeRTOS/source/timers.c \
               FreeRTOS/source/heap_3.c \
               FreeRTOS/portable/ThirdParty/GCC/Posix/port.c \
               FreeRTOS/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c

# Nesne Dosyaları Listesi (Object Files)
OBJS = $(SRCS_MAIN:.c=.o) \
       src/tasks.o \
       $(FREERTOS_SRC:.c=.o)

TARGET = freertos_sim

# Ana Hedef
all: $(TARGET)

# Linkleme (Birleştirme) Aşaması
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lrt

# Özel Kural: src/tasks.c dosyasını derlerken çakışmayı önle
src/tasks.o: src/tasks.c
	$(CC) $(CFLAGS) -c $< -o $@

# Genel Kural: Diğer .c dosyaları için
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Temizlik
clean:
	rm -f src/*.o FreeRTOS/source/*.o FreeRTOS/portable/ThirdParty/GCC/Posix/*.o FreeRTOS/portable/ThirdParty/GCC/Posix/utils/*.o $(TARGET)