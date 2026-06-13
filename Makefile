CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=gnu11
TARGET = bucket_sort_seq
SRCS = src/main.c src/bucket_sort.c src/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)