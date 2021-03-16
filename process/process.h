#ifndef PONG_PROCESS_H
#define PONG_PROCESS_H

#include "logger.h"
#include "../config/config.h"

#define MAX_CLIENTS 10

struct server_t {
    pid_t *workers;
    int nworkers;
    int master_socket;
    struct logger_context *l_context;
};

struct client_data {
    struct context *clients[MAX_CLIENTS];
    int count;
};

struct process_t {
    struct client_data client_socket;
    fd_set *readfds;
    fd_set *writefds;
    pid_t pid;
    struct logger_context *l_context;
    int running;
};

struct process_t *init_process(struct logger_context *l_context, char *server_name);
void close_process(struct process_t *process);

void run_process(struct process_t *process, int master_socket, struct sockaddr_in server_addr, char *server_name);

void body(int master_socket, struct sockaddr_in server_addr, struct logger_context *l_context, char *server_name);

pid_t create_process(struct server_t *server, struct sockaddr_in server_addr, Config *cfg);

int create_processes(struct server_t *server, struct sockaddr_in server_addr, Config *cfg);

#endif //PONG_PROCESS_H
