CC = gcc
STD = -std=gnu99

CFLAGS = $(shell pkg-config --cflags glib-2.0)

PF = -lpthread -lcunit $(shell pkg-config --libs glib-2.0)

FILES = config/config.c config/inih/ini.c context/context.c fsm/socket.c io_buf/io_buf.c process/process.c process/logger.c sds/sds.c

all: clean release

release: server_r
debug: server_d

server_r:
	$(CC) $(STD) $(CFLAGS) $(FILES) main.c -o server.out $(PF)
server_d:
	$(CC) $(STD) $(CFLAGS) $(FILES) main.c -o server.out $(PF)

clean:
	rm -rf *.o server.out
