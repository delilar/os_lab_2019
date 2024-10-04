CC = gcc
CFLAGS = -Wall -Wno-format -pthread
TARGET = parallel_sum
LIBS = -lpthread

# Файлы исходного кода
SRCS = parallel_sum.c sum_lib.c utils.c
# Сгенерированные объектные файлы
OBJS = $(SRCS:.c=.o)

# Цель по умолчанию
all: $(TARGET)

# Правило для сборки исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Правило для сборки объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для очистки сгенерированных файлов
clean:
	rm -f $(OBJS) $(TARGET)

# Рекурсивная очистка
.PHONY: all clean
