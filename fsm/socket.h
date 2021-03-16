//
// Created by zelenova-yu on 08.12.2020.
//

#ifndef PONG_SOCKET_H
#define PONG_SOCKET_H

#include "../context/context.h"
#include "../process/process.h"

#define REPLY_HELLO "220 SMTP myserver.ru ready"
#define REPLY_EHLO "250-RSET"

struct mail_data {
    sds sender;
    sds recipient;
    sds text;
};

int check_connection(struct context *ctx, struct process_t *process);

void main_handler(struct context *ctx, struct process_t *process, int is_read, int is_write);

int send_greetings(struct context *ctx, struct process_t *process);

int send_data(struct context *ctx, struct process_t *process);

int receive_data(struct context *ctx, struct process_t *process);

int detect_keyword(sds string, struct context *ctx);

void key_switcher(sds string, struct context *ctx);

void HELO_handler(sds string, struct context *ctx);

void EHLO_handler(sds string, struct context *ctx);

void MAIL_handler(sds string, struct context *ctx);

void RCPT_handler(sds string, struct context *ctx);

void DATA_handler(struct context *ctx);

void TEXT_handler(sds string, struct context *ctx);

void QUIT_handler(struct context *ctx);

void RSET_handler(struct context *ctx);

void ERROR_handler(struct context *ctx);

void create_file(struct context *ctx);

int check_user(sds user_mail, char *server_name);

#endif //PONG_SOCKET_H
