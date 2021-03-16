//
// Created by zelenova-yu on 13.12.2020.
//

#ifndef PONG_CONFIG_H
#define PONG_CONFIG_H

#include <string.h>
#include <stdlib.h>

typedef struct {
    int processes;
    char* logger_file;
    int log_level;
    char* name;
    int port;
} Config;

#define CFG_MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

static int cfg_handler(void* data, const char* section, const char* name, const char* value) {
    Config* cfg = (Config*)data;

    if (CFG_MATCH("multiprocessing", "count"))
        cfg->processes = atoi(value);
    else if (CFG_MATCH("logging", "level"))
        cfg->log_level = atoi(value);
    else if (CFG_MATCH("logging", "file"))
        cfg->logger_file = strdup(value);
    else if (CFG_MATCH("server", "name"))
        cfg->name = strdup(value);
    else if (CFG_MATCH("server", "port"))
        cfg->port = atoi(value);
    else
        return 0;

    return 1;
}

#endif //PONG_CONFIG_H
