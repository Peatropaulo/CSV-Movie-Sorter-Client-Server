CCFLAGS = -ggdb -Wall

all: server 

server: sorter_server.c sorter.c
	gcc $(CCFLAGS) -lpthread -o sc sorter_server.c sorter_server.h sorter.c

clean:
	rm -rf server

