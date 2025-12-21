CC = gcc
CFLAGS = -Wall -Wextra -g -I. -I./src -I./FreeRTOS/include -I./FreeRTOS/portable/ThirdParty/GCC/Posix -I./FreeRTOS/portable/ThirdParty/GCC/Posix/utils -D_CONSOLE_MODE

ifeq ($(OS), Windows_NT)
    LDFLAGS = -lpthread
else
    UNAME_S := $(shell uname -s)
    LDFLAGS = -lpthread
    ifeq ($(UNAME_S), Linux)
        LDFLAGS += -lrt
    endif
endif

SRCS = src/main.c src/scheduler.c src/hooks.c src/tasks.c \
       FreeRTOS/source/tasks.c FreeRTOS/source/queue.c FreeRTOS/source/list.c \
       FreeRTOS/source/timers.c FreeRTOS/source/heap_3.c \
       FreeRTOS/portable/ThirdParty/GCC/Posix/port.c \
       FreeRTOS/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c

OBJS = $(SRCS:.c=.o)
TARGET = freertos_sim

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
