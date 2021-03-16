#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "sys/socket.h"
#include "netinet/in.h"
#include "string.h"
#include "process/process.h"
#include "process/logger.h"
#include "config/config.h"
#include "config/inih/ini.h"

struct server_t *server;

void graceful_exit(int code) {
    printf("Server: closed");
    free(server->workers);

    if (close(server->master_socket) != 0)
        perror("close error");

    FreeLoggerContext(server->l_context);
    exit(code);
}

int main(int argc, char *argv[]) {
    server = malloc(sizeof(struct server_t));
    const char config[] = "config.ini";
    Config* cfg; int code = 0;
    if ((code = ini_parse(config, cfg_handler, cfg)) != 0) {
        printf("[ini_parse]: returned %d\n", code);
        return 0;
    }

    server->nworkers = cfg->processes;
    server->workers = malloc(sizeof(*(server->workers)) * server->nworkers);
    server->l_context = calloc(1, sizeof(struct logger_context));

    key_t l_key = ftok(MQ_LOGGER_PREFIX, 'l');
    int mq_logger = msgget(l_key, 0666 | IPC_CREAT);
    if ((server->l_context->data = NewLoggerData(cfg->logger_file, cfg->log_level, mq_logger)) == NULL) {
        fprintf(stderr, "NewLoggerData failed\n");
        graceful_exit(SIGINT);
    }

    struct sockaddr_in server_addr;

    // try create master
    if ((server->master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socked failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server->master_socket, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        perror("setsocksopt");
        exit(EXIT_FAILURE);
    }

    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    memset(&server_addr, '0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(cfg->port);

    // bind to localhost:PORT
    if (bind(server->master_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Listener on port %d \n", cfg->port);
    // max 10 connections for master
    if (listen(server->master_socket, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&server->l_context->thread, NULL, loggerEntrypoint, server->l_context->data)) {
        fprintf(stderr, "pthread_create failed for logger thread\n");
        ServerShutdown();
        graceful_exit(SIGINT);

        return 0;
    }

    int nump = create_processes(server, server_addr, cfg);
    if (nump == server->nworkers) {
        signal(SIGINT, graceful_exit);

        while (1) {
            sleep(5);
        }
    }

    //body(server->master_socket, server_addr, server->l_context, cfg->name);

    return 0;
}
