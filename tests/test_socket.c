//
// Created by zelenova-yu on 20.12.2020.
//
#include "CUnit/CUnit.h"
#include "../fsm/socket.h"
#include "../fsm/server-fsm.h"

void test_check_connection_correct(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct logger_context *l_context = calloc(1, sizeof(struct logger_context));
    key_t l_key = ftok(MQ_LOGGER_PREFIX, 'l');
    int mq_logger = msgget(l_key, 0666 | IPC_CREAT);
    l_context->data = NewLoggerData("/tmp/test", 0, mq_logger);

    CU_ASSERT_NOT_EQUAL(l_context->data, NULL);

    struct context* ctx = ctx_new(0, addr, server_name);
    struct process_t *proc = init_process(l_context, server_name);

    int status = check_connection(ctx, proc);
    CU_ASSERT_EQUAL(status, 0);

    ctx_close(ctx);
    close_process(proc);
    free(l_context);
}

void test_check_greetings_correct(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct logger_context *l_context = calloc(1, sizeof(struct logger_context));
    key_t l_key = ftok(MQ_LOGGER_PREFIX, 'l');
    int mq_logger = msgget(l_key, 0666 | IPC_CREAT);
    l_context->data = NewLoggerData("tmp/test", 0, mq_logger);

    CU_ASSERT_NOT_EQUAL(l_context->data, NULL);

    struct context* ctx = ctx_new(0, addr, server_name);
    struct process_t *proc = init_process(l_context, server_name);

    int status = send_greetings(ctx, proc);
    CU_ASSERT_EQUAL(status, 1);
    CU_ASSERT_EQUAL(ctx->current_step, SMTP_SERVER_FSM_ST_WAIT);

    ctx_close(ctx);
    close_process(proc);
    free(l_context);
}

void test_check_send_correct(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct logger_context *l_context = calloc(1, sizeof(struct logger_context));
    key_t l_key = ftok(MQ_LOGGER_PREFIX, 'l');
    int mq_logger = msgget(l_key, 0666 | IPC_CREAT);
    l_context->data = NewLoggerData("tmp/test", 0, mq_logger);

    CU_ASSERT_NOT_EQUAL(l_context->data, NULL);

    struct context* ctx = ctx_new(0, addr, server_name);
    struct process_t *proc = init_process(l_context, server_name);
    ctx_push_msg(ctx, "test");

    int status = send_data(ctx, proc);
    CU_ASSERT_EQUAL(status, 1);
    ctx_close(ctx);
    close_process(proc);
    free(l_context);
}

void test_check_receive_correct(void) {
    char* server_name = "test.com";
    struct server_addr addr;
    struct logger_context *l_context = calloc(1, sizeof(struct logger_context));
    key_t l_key = ftok(MQ_LOGGER_PREFIX, 'l');
    int mq_logger = msgget(l_key, 0666 | IPC_CREAT);
    l_context->data = NewLoggerData("tmp/test", 0, mq_logger);

    CU_ASSERT_NOT_EQUAL(l_context->data, NULL);

    struct context* ctx = ctx_new(10, addr, server_name);
    struct process_t *proc = init_process(l_context, server_name);

    int status = receive_data(ctx, proc);
    CU_ASSERT_EQUAL(status, 0);
    ctx_close(ctx);
    close_process(proc);
    free(l_context);
}

