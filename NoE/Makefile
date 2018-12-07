CCFLAGS = -ggdb -Wall

all: server 

server: sorter_server.c sort_server.h sorterQuickStruct.c
	gcc $(CCFLAGS) -pthread -o server sorter_server.c sort_server.h 

clean:
	rm -rf server
