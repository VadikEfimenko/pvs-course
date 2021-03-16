//
// Created by zelenova-yu on 07.12.2020.
//

#ifndef PONG_LOGGER_H
#define PONG_LOGGER_H

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <errno.h>

#define MQ_LOGGER_PREFIX "/tmp/"
#define log_trace(...) log_log(TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(WARNING,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define MSG_SIZE 512

struct log_message {
    long int level;
    int th;
    const char *file;
    int line;
    char text[MSG_SIZE];
    time_t time;
};

struct worker_message {
    long int type;
    char text[MSG_SIZE];
};

struct worker_data {
    int id;
    int sendTime;
    int retryTime;
    int mqTasks;
    int mqLogger;
};

enum LOGLEVEL {
    TRACE = 1, // initialize as hack for msgsnd message type
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

static const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
        "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

struct logger_data {
    int mq;
    int level;
    char *file;
};

struct logger_context {
    pthread_t thread;
    struct logger_data *data;
};

struct logger_data *NewLoggerData(char *file, int level, int mq);

void FreeLoggerContext(struct logger_context *ctx);

int log_log(int level, const char *file, int line, int lfd, int th, const char *msg);

void *loggerEntrypoint(void *args);

void *workerEntrypoint(void *args);

void ServerShutdown();

#endif //PONG_LOGGER_H
