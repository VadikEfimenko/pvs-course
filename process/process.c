//
// Created by zelenova-yu on 30.11.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/procfs.h>

#include "sys/socket.h"
#include "netinet/in.h"
#include "string.h"
#include "../context/context.h"
#include "process.h"
#include "logger.h"

char msg[512];

struct process_t *init_process(struct logger_context *l_context, char *server_name) {
    struct process_t *process = (struct process_t *) malloc(sizeof(struct process_t));
    process->readfds = malloc(sizeof(process->readfds));
    process->writefds = malloc(sizeof(process->writefds));
    process->pid = getpid();
    process->running = 1;
    process->l_context = l_context;

    struct server_addr *ip_s = malloc(MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        process->client_socket.clients[i] = ctx_new(0, ip_s[i], server_name);
    }

    return process;
}

void close_process(struct process_t *process) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        ctx_close(process->client_socket.clients[i]);
    }
    free(process->readfds);
    free(process->writefds);

    free(process);
}

void run_process(struct process_t *process, int master_socket, struct sockaddr_in server_addr, char *server_name) {
    int max_socket_desc, socket_desc, activity, connfd, server_addr_len = sizeof(server_addr);
    while (process->running) {
        FD_ZERO(process->readfds);
        FD_ZERO(process->writefds);

        // add master socket
        FD_SET(master_socket, process->readfds);
        FD_SET(master_socket, process->writefds);
        max_socket_desc = master_socket;

        // add child sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            socket_desc = process->client_socket.clients[i]->sfd;

            if (socket_desc > 0) {
                FD_SET(socket_desc, process->readfds);
                FD_SET(socket_desc, process->writefds);
            }
            if (socket_desc > max_socket_desc)
                max_socket_desc = socket_desc;
        }

        activity = select(max_socket_desc + 1, process->readfds, process->writefds, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            log_error(process->l_context->data->mq, getpid(), "Select error");

        if (activity == 0) {
            sprintf(msg, "Worker(%d): select timeout", getpid());
            log_warn(process->l_context->data->mq, getpid(), msg);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!FD_ISSET(process->client_socket.clients[i]->sfd, process->readfds))
                    process->client_socket.clients[i] = ctx_new(0, process->client_socket.clients[i]->sa, server_name);
            }
        }

        if (FD_ISSET(master_socket, process->readfds)) {
            if ((connfd = accept(master_socket, (struct sockaddr *) &server_addr, (socklen_t *) &server_addr_len)) <
                0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            sprintf(msg, "New connection, socket fd is %d, ip is : %s, port: %d, process: %d", connfd,
                    inet_ntoa(server_addr.sin_addr),
                    ntohs(server_addr.sin_port), process->pid);
            log_info(process->l_context->data->mq, getpid(), msg);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (process->client_socket.clients[i]->sfd == 0) {
                    process->client_socket.clients[i] = ctx_new(connfd, process->client_socket.clients[i]->sa, server_name);
                    send_greetings(process->client_socket.clients[i], process);
                    sprintf(msg, "Adding to list of sockets as %d, connfd: %d", i, connfd);
                    log_info(process->l_context->data->mq, getpid(), msg);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            struct context *ctx = process->client_socket.clients[i];

            if (FD_ISSET(ctx->sfd, process->readfds) || FD_ISSET(ctx->sfd, process->writefds)) {
                main_handler(ctx, process, FD_ISSET(ctx->sfd, process->readfds), FD_ISSET(ctx->sfd, process->writefds));
            }
        }
    }
}

void body(int master_socket, struct sockaddr_in server_addr, struct logger_context *l_context, char *server_name) {
    struct process_t *proc = init_process(l_context, server_name);
    if (proc != NULL) {
        sprintf(msg, "Worker(%d): started work", getpid());
        log_info(l_context->data->mq, getpid(), msg);
        run_process(proc, master_socket, server_addr, server_name);
        close_process(proc);
    }
    sprintf(msg, "Worker(%d): process killed", getpid());
    log_info(l_context->data->mq, getpid(), msg);
    kill(getpid(), SIGTERM);
}

pid_t create_process(struct server_t *server, struct sockaddr_in server_addr, Config *cfg) {
    pid_t pid = fork();
    switch (pid) {
        case 0:
            body(server->master_socket, server_addr, server->l_context, cfg->name);
            break;
        case -1:
            sprintf(msg, "Server(%d): fork() failed", getpid());
            log_error(server->l_context->data->mq, getpid(), msg);
            break;
        default:
            sprintf(msg, "Server(%d): create proccess(%d)", getpid(), pid);
            log_info(server->l_context->data->mq, getpid(), msg);
            break;
    }
    return pid;
}

int create_processes(struct server_t *server, struct sockaddr_in server_addr, Config *cfg) {
    int num = 0;
    for (int i = 0; i < server->nworkers; i++) {
        lwpid_t tmp = create_process(server, server_addr, cfg);
        if (tmp != -1) {
            num++;
            server->workers[i] = tmp;
        }
    }
    return num;
}
