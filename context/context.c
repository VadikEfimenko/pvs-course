//
// Created by yurii-kalugin on 03.11.2020.
//

#include <unistd.h>

#include "../io_buf/io_buf.h"
#include "context.h"
#include "../fsm/server-fsm.h"

int parse_ipv4_address(const char* input, struct server_addr* addr) {
    struct in_addr ipv4;
    if (inet_pton(AF_INET, input, &ipv4) == 1) {
        addr->ip = ipv4;
        return 1;
    }

    return 0;
}

int parse_port(const char* input) {
    char* endptr;
    int port = (int)strtoul(input, &endptr, 10);

    if (port > 0 && *endptr == '\0')
        return port;

    return 0;
}

// ipv4_address
// ipv4_address:port
int parse_address(const char* input, struct server_addr* result) {
    char* ptr = strrchr(input, ':');

    if (ptr != NULL && ptr > input) {
        int port = parse_port((ptr + 1));
        if (!port) {
            // failed to parse port
            return 0;
        }
        result->port = port;

        char* copy = strdup(input);
        long int end = ptr - input;
        copy[end] = '\0';

        struct in_addr ipv4;

        int success = parse_ipv4_address(copy, result);
        if (!success) {
            // failed to parse ipv4
            return  0;
        }
        return 1;
    }


    return parse_ipv4_address(input, result);
}

// Return NULL on fail
struct context* ctx_new(int sfd, struct server_addr addr, char *server_name) {
    struct context* ctx = (struct context*)malloc(sizeof(struct context));
    if (!ctx) { // ctx == NULL
        return NULL;
    }

    struct io_buf* reader = io_buf_new();
    if (!reader) { // reader == NULL
        free(ctx);
        return NULL;
    }
    ctx->reader = reader;

    struct io_buf* writer = io_buf_new();
    if (writer == NULL) { // writer == NULL
        free(ctx);
        io_buf_destroy(reader);
        return NULL;
    }

    ctx->writer = writer;
    ctx->sa = addr;
    ctx->sfd = sfd;
    ctx->current_step = SMTP_SERVER_FSM_ST_INIT_S;
    ctx->mail = malloc(sizeof(struct mail_data));
    ctx->mail->text = sdsempty();
    ctx->mail->recipient = sdsempty();
    ctx->mail->sender = sdsempty();
    ctx->server_name = server_name;

    return ctx;
}

void ctx_close(struct context* ctx) {
    // close tcp socket
    close(ctx->sfd);
    // delete according messages
    io_buf_destroy(ctx->reader);
    io_buf_destroy(ctx->writer);

    sdsfree(ctx->mail->text);
    sdsfree(ctx->mail->recipient);
    sdsfree(ctx->mail->sender);

    free(ctx->mail);
    free(ctx);
}

// process reading from socket
unsigned int ctx_read(struct context* ctx) {
    return io_buf_read(ctx->reader, ctx->sfd);
}

// process writing to socket
unsigned int ctx_write(struct context* ctx) {
    unsigned int is_empty = io_buf_is_empty(ctx->writer);
    if (!is_empty) {
        return io_buf_write(ctx->writer, ctx->sfd);
    }

    return 0;
}

sds ctx_pop_msg(struct context* ctx) {
    return io_buf_pop(ctx->reader);
}

void ctx_push_msg(struct context* ctx, const char* msg) {
    io_buf_push(ctx->writer, msg);
}