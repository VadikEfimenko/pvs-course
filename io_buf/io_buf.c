//
// Created by yurii-kalugin on 03.11.2020.
//

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include "gmodule.h"

#include "../sds/sds.h"

#include "io_buf.h"


struct io_buf* io_buf_new() {
    struct io_buf* b_ = (struct io_buf*)malloc(sizeof(struct io_buf));

    if (b_ == NULL) return NULL;

    GQueue* deque = g_queue_new();
    if (!deque) {
        free(b_);
        return NULL;
    }

    sds buffer = sdsempty();
    if (!buffer) {
        free(b_);
        g_queue_free(deque);
        return NULL;
    }

    b_->full_msg_s = deque;
    b_->unfinished_msg = buffer;

    return b_;
}

// clear all allocated memory
void io_buf_destroy(struct io_buf* buffer) {
    sdsfree(buffer->unfinished_msg);
    // deque should free all pointed memory
    for (GList* iterator = buffer->full_msg_s->head; iterator; iterator = iterator->next) {
        sdsfree(iterator->data);
        //free(iterator);
    }
    // free deque itself
    g_queue_free(buffer->full_msg_s);
    // free buffer
    free(buffer);
}

unsigned int io_buf_is_empty(struct io_buf* buffer_) {
    // return dq_is_empty(buffer_->full_msg_s);
    return g_queue_is_empty(buffer_->full_msg_s);
}

// Read data from SFD by chunk with size of READ_SIZE.
// Stores finished messages (ending with MSG_TERM) into full_msg_s of buffer_.
// The rest is stored into unfinished_msg of buffer_.
unsigned int io_buf_read(struct io_buf* buffer_, int sfd) {
    char readbuffer[READ_SIZE];
    int nbytes = read(sfd, readbuffer, sizeof(readbuffer));

    if (nbytes <= 0)
        return 0;

    // splitted strings
    sds* tokens;
    sds received = sdsnewlen(readbuffer, nbytes);
    sds line = sdscat(buffer_->unfinished_msg, received);
    // number of tokens
    int t_count;
    tokens = sdssplitlen(line, sdslen(line), MSG_TERM, MSG_TERM_LEN, &t_count);

    // add full messages to deque
    for (int i = 0; i < t_count - 1; i++) {
        // make copy because deque stores pointers to data
        g_queue_push_tail(buffer_->full_msg_s, sdsdup(tokens[i]));
    }
    // the last one is unfinished add to buffer
    if (tokens[t_count - 1]) {
        buffer_->unfinished_msg = sdsdup(tokens[t_count - 1]);
    }

    // free dynamic strings
    sdsfreesplitres(tokens, t_count);
    return t_count - 1;
}

// Writes messages from unfinished_msg and full_msg_s into SFD by chunk with size WRITE_SIZE.
// The excess part  of last message stored into unfinished_msg.
unsigned int io_buf_write(struct io_buf* buffer_, int sfd) {
    sds string = g_queue_pop_head(buffer_->full_msg_s);
    string = sdscat(string, MSG_TERM);
    printf("Sending: %s\n", string);
    int nbytes = write(sfd, string, sdslen(string));

    if (nbytes == 0) {
        g_queue_push_head(buffer_->full_msg_s, string);
    }

    sdsfree(string);

    return nbytes;
}

sds io_buf_pop(struct io_buf* buffer_) {
    return g_queue_pop_head(buffer_->full_msg_s);
}

void io_buf_push(struct io_buf* buffer_, const char* msg) {
    sds msg_ = sdsnew(msg);
    //msg_ = sdscat(msg_, MSG_TERM);
    g_queue_push_tail(buffer_->full_msg_s, msg_);
}