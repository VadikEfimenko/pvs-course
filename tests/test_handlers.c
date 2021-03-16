//
// Created by zelenova-yu on 18.12.2020.
//
#include "CUnit/CUnit.h"
#include "../fsm/socket.h"
#include "../fsm/server-fsm.h"

void test_correct_automata(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("HELO localhost");
    ctx->current_step = SMTP_SERVER_FSM_ST_WAIT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_MAIL);

    sds msg2 = sdsnew("MAIL FROM test@test.com");
    key_switcher(msg2, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_RCPT);

    sds msg3 = sdsnew("RCPT TO test2@test.com");
    key_switcher(msg3, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_RCPT);

    sds msg4 = sdsnew("DATA");
    key_switcher(msg4, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DATA);

    sds msg5 = sdsnew(".askjfhjskhrgf");
    key_switcher(msg5, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_TEXT);

    sds msg6 = sdsnew(".");
    key_switcher(msg6, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_CLOSE);

    sds msg7 = sdsnew("QUIT");
    key_switcher(msg7, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    sdsfree(msg2);
    sdsfree(msg3);
    sdsfree(msg4);
    sdsfree(msg5);
    sdsfree(msg6);
    sdsfree(msg7);
    ctx_close(ctx);
}

void test_correct_HELO(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("HELO localhost");
    ctx->current_step = SMTP_SERVER_FSM_ST_WAIT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_MAIL);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_incorrect_HELO(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("HELO");
    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_EHLO(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("EHLO localhost");
    ctx->current_step = SMTP_SERVER_FSM_ST_WAIT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_MAIL);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_incorrect_EHLO(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("EHLO");
    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_MAIL(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("MAIL FROM pupa@luma.com");
    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_RCPT);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_incorrect_MAIL(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("MAIL pupa@luma.com");
    ctx->current_step = SMTP_SERVER_FSM_ST_MAIL;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_RCPT(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("RCPT TO pupa@luma.com");
    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_RCPT);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_incorrect_RCPT(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("RCPT pupa@test.com");
    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_DATA(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("DATA");
    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DATA);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_incorrect_DATA(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("DATA hello world");
    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_TEXT(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("kjdjfghksjhdg");
    ctx->current_step = SMTP_SERVER_FSM_ST_DATA;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_TEXT);

    sds msg2 = sdsnew(".");
    key_switcher(msg2, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_CLOSE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_QUIT(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("QUIT");
    ctx->current_step = SMTP_SERVER_FSM_ST_CLOSE;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_RSET(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("RSET");
    ctx->current_step = SMTP_SERVER_FSM_ST_TEXT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_WAIT);

    sdsfree(msg1);
    ctx_close(ctx);
}

void test_correct_ERROR(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct context* ctx = ctx_new(0, addr, server_name);

    sds msg1 = sdsnew("rthhgfdsgh");
    ctx->current_step = SMTP_SERVER_FSM_ST_RCPT;
    key_switcher(msg1, ctx);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_DONE);

    sdsfree(msg1);
    ctx_close(ctx);
}

