CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = main.o erproc.o
TARGET = ipv4_chat_broadcast

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lpthread

main.o: main.c erproc.h
	$(CC) $(CFLAGS) -c main.c

erproc.o: erproc.c erproc.h
	$(CC) $(CFLAGS) -c erproc.c

clean:
	rm -f $(OBJ) $(TARGET)