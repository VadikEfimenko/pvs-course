//
// Created by zelenova-yu on 07.12.2020.
//

#include "logger.h"

bool th_worker_running = true;
bool th_logger_running = true;
bool th_master_running = true;

struct logger_data *NewLoggerData(char *file, int level, int mq) {
    struct logger_data *ldata = calloc(1, sizeof(struct logger_data));

    if (!ldata) return NULL;

    ldata->file = file;
    ldata->level = level;
    ldata->mq = mq;
    return ldata;
}

void ServerShutdown() {
    th_master_running = false;
    th_logger_running = false;
    th_worker_running = false;
}

void FreeLoggerContext(struct logger_context *ctx) {
    if (msgctl(ctx->data->mq, IPC_RMID, NULL) != 0) {
        fprintf(stderr, "[LOGGER]: error during closing mq: '%s'\n", strerror(errno));
    }
    free(ctx->data);
    free(ctx);
}

int log_log(int level, const char *file, int line, int lfd, int th, const char *msg) {
    struct log_message message = {
            .level = level,
            .time = time(NULL),
            .th = th,
            .file = file,
            .line = line,
    };
    strcpy(message.text, msg);


    return msgsnd(lfd, &message, sizeof(message), 0x00);
}

void *loggerEntrypoint(void *args) {
    struct logger_data *ldata = (struct logger_data *) args;

    struct log_message message;
    struct timespec sleep = {1, 0};
    while (th_logger_running) {
        memset(&message, 0x00, sizeof(struct log_message));

        if (msgrcv(ldata->mq, &message, sizeof(message), 0, 0x00) < 0) {
            fprintf(stderr, "msgrcv failed for logger\n");
            th_logger_running = false;
        }

        if (message.level >= ldata->level) {
            char sTime[26];
            struct tm *tm_info;
            tm_info = localtime(&message.time);

            strftime(sTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            int level = (int) message.level - 1;

            switch (message.th) {
                case -1:
                    fprintf(stdout, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
                            sTime, level_colors[level], level_strings[level],
                            message.file, message.line);
                    fprintf(stdout, "<MAIN THREAD>: %s", message.text);
                    fprintf(stdout, "\n");
                    break;
                default:
                    fprintf(stdout, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
                            sTime, level_colors[level], level_strings[level],
                            message.file, message.line);
                    fprintf(stdout, "<THREAD %d>: %s", message.th, message.text);
                    fprintf(stdout, "\n");
                    break;
            }
        }
    }

    free(ldata);
    return NULL;
}

void *workerEntrypoint(void *args) {
    struct worker_data *wdata = (struct worker_data *) args;

    struct worker_message message;
    struct timespec sleep = {2, 0};

    while (th_worker_running) {
        memset(&message, 0x00, sizeof(struct worker_message));

        msgrcv(wdata->mqTasks, &message, sizeof(message), 0, 0x00);
        log_trace(wdata->mqLogger, wdata->id, message.text);
        log_debug(wdata->mqLogger, wdata->id, message.text);
        log_info(wdata->mqLogger, wdata->id, message.text);
        log_warn(wdata->mqLogger, wdata->id, message.text);
        log_error(wdata->mqLogger, wdata->id, message.text);
        log_fatal(wdata->mqLogger, wdata->id, message.text);

        nanosleep(&sleep, NULL);
    }

    free(wdata);
    return NULL;
}
