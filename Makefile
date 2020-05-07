CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread
OBJS =

.PHONY: all clean

all: main

main: main
	$(CC) -o main main.c $(LFLAGS) $(LIBS)

clean: 
	rm -f main

