//
// Created by zelenova-yu on 08.12.2020.
//
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sys/socket.h"
#include "string.h"
#include "../context/context.h"
#include "socket.h"
#include "server-fsm.h"

char msg[256];

void main_handler(struct context *ctx, struct process_t *process, int is_read, int is_write) {
    if (check_connection(ctx, process))
        return;

    if (is_read)
        receive_data(ctx, process);

    if (is_write)
        send_data(ctx, process);
}

int check_connection(struct context *ctx, struct process_t *process) {
    if (recv(ctx->sfd, NULL, 1, MSG_PEEK | MSG_DONTWAIT) == 0 || ctx->current_step == SMTP_SERVER_FSM_ST_DONE) {
        sprintf(msg, "Worker(%d): clear socket %d", getpid(), ctx->sfd);
        log_info(process->l_context->data->mq, getpid(), msg);
        ctx_close(ctx);
        ctx = ctx_new(0, ctx->sa, ctx->server_name);
        return 1;
    }
    return 0;
}

int send_greetings(struct context *ctx, struct process_t *process) {
    if (ctx->current_step == SMTP_SERVER_FSM_ST_INIT_S) {
        char buf[256];
        sprintf(buf, REPLY_HELLO);
        ctx_push_msg(ctx, buf);
        ctx->current_step = SMTP_SERVER_FSM_ST_WAIT;
        return 1;
    }
    return 0;
}

int send_data(struct context *ctx, struct process_t *process) {
    int nbytes = (int) ctx_write(ctx);
    if (nbytes) {
        sprintf(msg, "Sent: %d bytes", nbytes);
        log_info(process->l_context->data->mq, getpid(), msg);
        return 1;
    }
    return 0;
}

int receive_data(struct context *ctx, struct process_t *process) {
    int msg_count = (int) ctx_read(ctx);

    if (!msg_count)
        return 0;

    for (int idx = 0; idx < msg_count; idx++) {
        sds string = ctx_pop_msg(ctx);
        if (string) {
            sprintf(msg, "Received: %s", string);
            log_info(process->l_context->data->mq, getpid(), msg);
        }
        key_switcher(string, ctx);
    }
    return 1;
}

int detect_keyword(sds string, struct context *ctx) {
    char *sep = " ";
    int tcount = 0, code = 100;
    sds *keyword = sdssplitlen(string, sdslen(string), sep, strlen(sep), &tcount);

    switch (tcount) {
        case 1: {
            if (strcmp(*keyword, "DATA") == 0)
                code = SMTP_SERVER_FSM_EV_DATA;
            else if (strcmp(*keyword, "QUIT") == 0)
                code = SMTP_SERVER_FSM_EV_QUIT;
            else if (strcmp(*keyword, "RSET") == 0)
                code = SMTP_SERVER_FSM_EV_RSET;
            else if (ctx->current_step == SMTP_SERVER_FSM_ST_DATA || ctx->current_step == SMTP_SERVER_FSM_ST_TEXT)
                code = SMTP_SERVER_FSM_EV_TEXT;
            break;
        }
        case 3: {
            if (strcmp(*keyword, "MAIL") == 0 && (strcmp(keyword[1], "FROM") == 0 || strcmp(keyword[1], "FROM:") == 0))
                code = SMTP_SERVER_FSM_EV_MAIL;
            else if (strcmp(*keyword, "RCPT") == 0 && (strcmp(keyword[1], "TO") == 0 || strcmp(keyword[1], "TO:") == 0))
                code = SMTP_SERVER_FSM_EV_RCPT;
            else if (ctx->current_step == SMTP_SERVER_FSM_ST_DATA || ctx->current_step == SMTP_SERVER_FSM_ST_TEXT)
                code = SMTP_SERVER_FSM_EV_TEXT;
            break;
        }
        case 2: {
            if (strcmp(*keyword, "HELO") == 0)
                code = SMTP_SERVER_FSM_EV_HELO;
            else if (strcmp(*keyword, "EHLO") == 0)
                code = SMTP_SERVER_FSM_EV_EHLO;
            else if (ctx->current_step == SMTP_SERVER_FSM_ST_DATA || ctx->current_step == SMTP_SERVER_FSM_ST_TEXT)
                code = SMTP_SERVER_FSM_EV_TEXT;
            break;
        }
        default:
            if (ctx->current_step == SMTP_SERVER_FSM_ST_DATA || ctx->current_step == SMTP_SERVER_FSM_ST_TEXT)
                code = SMTP_SERVER_FSM_EV_TEXT;
            break;
    }

    sdsfreesplitres(keyword, tcount);
    return code;
}

void key_switcher(sds string, struct context *ctx) {
    int key = detect_keyword(string, ctx);
    switch (key) {
        case SMTP_SERVER_FSM_EV_HELO:
            HELO_handler(string, ctx);
            break;
        case SMTP_SERVER_FSM_EV_EHLO:
            EHLO_handler(string, ctx);
            break;
        case SMTP_SERVER_FSM_EV_MAIL:
            MAIL_handler(string, ctx);
            break;
        case SMTP_SERVER_FSM_EV_RCPT:
            RCPT_handler(string, ctx);
            break;
        case SMTP_SERVER_FSM_EV_DATA:
            DATA_handler(ctx);
            break;
        case SMTP_SERVER_FSM_EV_TEXT:
            TEXT_handler(string, ctx);
            break;
        case SMTP_SERVER_FSM_EV_QUIT:
            QUIT_handler(ctx);
            break;
        case SMTP_SERVER_FSM_EV_RSET:
            RSET_handler(ctx);
            break;
        default:
            ERROR_handler(ctx);
            break;
    }
}

void HELO_handler(sds string, struct context *ctx) {
    if (ctx->current_step != SMTP_SERVER_FSM_ST_WAIT)
        ERROR_handler(ctx);
    char *sep = " ";
    int tcount = 0;
    sds *keyword = sdssplitlen(string, sdslen(string), sep, strlen(sep), &tcount);

    string = sdscatprintf(sdsempty(), "250 Hello my new friend - %s", keyword[1]);
    ctx_push_msg(ctx, string);

    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    sdsfreesplitres(keyword, tcount);
}

void EHLO_handler(sds string, struct context *ctx) {
    if (ctx->current_step != SMTP_SERVER_FSM_ST_WAIT)
        ERROR_handler(ctx);

    char *sep = " ";
    int tcount = 0;
    sds *keyword = sdssplitlen(string, sdslen(string), sep, strlen(sep), &tcount);

    string = sdscatprintf(sdsempty(), "250 Hello my new friend - %s", keyword[1]);
    ctx_push_msg(ctx, string);
    ctx_push_msg(ctx, REPLY_EHLO);

    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    sdsfreesplitres(keyword, tcount);
}

void MAIL_handler(sds string, struct context *ctx) {
    if (ctx->current_step != SMTP_SERVER_FSM_ST_MAIL && ctx->current_step != SMTP_SERVER_FSM_ST_CLOSE)
        ERROR_handler(ctx);

    if (ctx->current_step == SMTP_SERVER_FSM_ST_CLOSE) {
        create_file(ctx);
        sdsclear(ctx->mail->recipient);
        sdsclear(ctx->mail->sender);
        sdsclear(ctx->mail->text);
    }

    char *sep = " ";
    int tcount = 0;
    sds *keyword = sdssplitlen(string, sdslen(string), sep, strlen(sep), &tcount);

    ctx->mail->sender = sdsnew(keyword[2]);
    string = sdscatprintf(sdsempty(), "250 %s sender accepted", keyword[2]);
    ctx_push_msg(ctx, string);

    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    sdsfreesplitres(keyword, tcount);
}

void RCPT_handler(sds string, struct context *ctx) {
    if (ctx->current_step != SMTP_SERVER_FSM_ST_RCPT)
        ERROR_handler(ctx);

    char *sep = " ";
    int tcount = 0;
    sds *keyword = sdssplitlen(string, sdslen(string), sep, strlen(sep), &tcount);

    sds rcpt = sdsdup(keyword[2]);
    sdstrim(rcpt, "<>");

    if (check_user(rcpt, ctx->server_name)) {
        ctx->mail->recipient = sdscat(ctx->mail->recipient, rcpt);
        ctx->mail->recipient = sdscat(ctx->mail->recipient, ",");

        ctx_push_msg(ctx, "250 recipient accepted");
    } else {
        ctx_push_msg(ctx, "251 recipient does not belong to the domain");
    }

    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    sdsfree(rcpt);
    sdsfreesplitres(keyword, tcount);
}

void DATA_handler(struct context *ctx) {
    if (ctx->current_step == SMTP_SERVER_FSM_ST_RCPT) {
        ctx_push_msg(ctx, "354 Enter message, ending with \\\".\\\" on a line by itself\\r\\n");
        ctx->current_step = SMTP_SERVER_FSM_ST_DATA;
    } else
        ERROR_handler(ctx);
}

void TEXT_handler(sds string, struct context *ctx) {
    if (ctx->current_step == SMTP_SERVER_FSM_ST_TEXT || ctx->current_step == SMTP_SERVER_FSM_ST_DATA) {
        sds eof = sdsnew(".");

        if (sdscmp(string, eof) == 0) {
            ctx_push_msg(ctx, "250 message accepted for delivery");
            ctx->current_step = SMTP_SERVER_FSM_ST_CLOSE;
        } else {
            ctx->mail->text = sdscatsds(ctx->mail->text, string);
            ctx->current_step = SMTP_SERVER_FSM_ST_TEXT;
        }

        sdsfree(eof);
    } else
        ERROR_handler(ctx);
}

void QUIT_handler(struct context *ctx) {
    if (ctx->current_step != SMTP_SERVER_FSM_ST_CLOSE)
        ERROR_handler(ctx);
    ctx_push_msg(ctx, "221 closing connection");
    create_file(ctx);
    ctx->current_step = SMTP_SERVER_FSM_ST_DONE;
}

void RSET_handler(struct context *ctx) {
    ctx->current_step = SMTP_SERVER_FSM_ST_WAIT;
    ctx_push_msg(ctx, "250 reset connection");
    sdsclear(ctx->mail->text);
    sdsclear(ctx->mail->recipient);
    sdsclear(ctx->mail->sender);
}

void ERROR_handler(struct context *ctx) {
    ctx_push_msg(ctx, "503 Error");
    ctx->current_step = SMTP_SERVER_FSM_ST_DONE;
}

void create_file(struct context *ctx) {
    struct stat st = {0};
    char *sep = ",";
    int tcount = 0;
    sds *keyword = sdssplitlen(ctx->mail->recipient, sdslen(ctx->mail->recipient), sep, strlen(sep), &tcount);

    if (stat("../../emails", &st) == -1) {
        mkdir("../../emails", 0777);
    }

    for (int i = 0; i < tcount-1; i++) {
        sds path = sdscatprintf(sdsempty(), "../../emails/%s", keyword[i]);
        sds new_dir = sdscatprintf(sdsempty(), "%s/new", path);
        sds tmp_dir = sdscatprintf(sdsempty(), "%s/tmp", path);
        if (stat(path, &st) == -1) {
            mkdir(path, 0777);
            mkdir(new_dir, 0777);
            mkdir(tmp_dir, 0777);
        }

        time_t rawtime;
        char buffer[30];
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        sds filename = sdscatprintf(sdsempty(), "%s/%s_%s.txt", new_dir, ctx->mail->sender, buffer);
        FILE *fp = fopen(filename, "a");
        if (fp != NULL) {
            fprintf(fp, "MAIL FROM: %s\n", ctx->mail->sender);
            fprintf(fp, "RCPT TO: %s\n", keyword[i]);
            fprintf(fp, "%s", ctx->mail->text);
            fclose(fp);
        }
        sdsfree(path);
        sdsfree(new_dir);
        sdsfree(tmp_dir);
    }
    sdsfreesplitres(keyword, tcount);
}

int check_user(sds user_mail, char *server_name) {
    char *sep = "@";
    int tcount = 0;
    sds server = sdsnew(server_name);
    sds *keyword = sdssplitlen(user_mail, sdslen(user_mail), sep, strlen(sep), &tcount);

    if (tcount == 2) {
        if (sdscmp(keyword[1], server) == 0) {
            sdsfreesplitres(keyword, tcount);
            sdsfree(server);
            return 1;
        }
    }
    sdsfreesplitres(keyword, tcount);
    sdsfree(server);

    return 0;
}