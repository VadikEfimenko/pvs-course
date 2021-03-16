CC = gcc
STD = -std=gnu99

CFLAGS = $(shell pkg-config --cflags glib-2.0)

PF = -lpthread -lcunit $(shell pkg-config --libs glib-2.0)

FILES = test_handlers.c test_socket.c ../sds/sds.c ../context/context.c ../fsm/socket.c ../process/process.c ../process/logger.c ../io_buf/io_buf.c

all: clean release

release: server_r
debig: server_d

server_r:
	$(CC) $(STD) $(CFLAGS) $(FILES) test_main.c -o test_server.out $(PF)
server_d:
	$(CC) $(STD) $(CFLAGS) $(FILES) test_main.c -o test_server.out $(PF)

clean:
	rm -rf *.o server.out