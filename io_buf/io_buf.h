//
// Created by yurii-kalugin on 03.11.2020.
//

#ifndef PING_IO_BUF_H
#define PING_IO_BUF_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <arpa/inet.h>
#include "gmodule.h"

#include "../sds/sds.h"

#define READ_SIZE 512
#define WRITE_SIZE 512
#define MSG_TERM "\r\n"
#define MSG_TERM_LEN strlen(MSG_TERM)

// const char* MTERM = "\n";

struct io_buf {
    // Deque for ready to send finished messages
    GQueue* full_msg_s;
    // Buffer for unfinished message. Contains only one potential message.
    sds unfinished_msg;
};

struct io_buf* io_buf_new();
void io_buf_destroy(struct io_buf* buffer);
unsigned int io_buf_is_empty(struct io_buf* buffer_);

// Returns how many full messages has been read
unsigned int io_buf_read(struct io_buf* buffer_, int sfd);
// Returns how many bytes has been actually sent
unsigned int io_buf_write(struct io_buf* buffer_, int sfd);

// Pop returns copy of its own sds pointer and frees it. Do not forget to sdsfree() result.
sds io_buf_pop(struct io_buf* buffer_);
// Push makes its own copy of MSG, so MSG should be freed after
void io_buf_push(struct io_buf* buffer_, const char* msg);

#endif //PING_IO_BUF_H
