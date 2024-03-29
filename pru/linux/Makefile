CC = arm-linux-gnueabihf-gcc -pthread
CFLAGS = -Wall -Wextra -pedantic -std=c11
LDFLAGS = -lrt

TARGET = linux
SRCS = sharedMem-Linux.c accelerometer.c
OBJS = $(SRCS:.c=.o)
DEPS = sharedDataStruct.h accelerometer.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
