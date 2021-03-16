#ifndef PING_CONTEXT_H
#define PING_CONTEXT_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <arpa/inet.h>

#include "../io_buf/io_buf.h"
#include "../fsm/socket.h"

struct server_addr {
    struct in_addr ip;
    int port;
};

struct context {
    // server address
    struct server_addr sa;
    // Socket file descriptor (sfd) for associated socket
    int sfd;
    // Writing part
    struct io_buf* writer;
    // Reading part
    struct io_buf* reader;
    struct mail_data* mail;
    char *server_name;
    int current_step;
};

int parse_ipv4_address(const char* input, struct server_addr* addr);
int parse_port(const char* input);
int parse_address(const char* input, struct server_addr* result);

struct context* ctx_new(int sfd, struct server_addr addr, char *server_name);
void ctx_close(struct context* ctx);
// Reads messages from CTX.SFD and stores full messages into CTX.READER.FULL_MSG_S.
// Unfinished message if it exists is stored into CTX.READER.UNFINISHED_MSG
// Returns number of full messages.
unsigned int ctx_read(struct context* ctx);
// Writes messages from CTX.WRITER.FULL_MSG_S into CTX.SFD with blocks size of WRITE_SIZE.
// Left parts are stored into CTX.WRITER.FULL_MSG_S from head.
// Returns bytes (NOT MESSAGES) written.
unsigned int ctx_write(struct context* ctx);
// Get message from CTX.WRITER.FULL_MSG_S. Dot not forget ot use sdsfree().
sds ctx_pop_msg(struct context* ctx);
// Add MSG as full message to CTX.READER.FULL_MSG_S. Adds MSG_TERM at the end of the MSG.
void ctx_push_msg(struct context* ctx, const char* msg);

#endif //PING_CONTEXT_H
