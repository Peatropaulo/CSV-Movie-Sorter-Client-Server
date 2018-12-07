CFLAGS= -ggdb3 -std=gnu11 -Wall -Wextra #-Werror
FILES= sorter.c sorter.h

main: $(FILES)
	gcc -pthread $(CFLAGS) -D TPOOL_SIZE=4 -o sorter_client $(FILES)

clean:
	find -type f -name '*-sorted*' -delete
	rm sorter_client
