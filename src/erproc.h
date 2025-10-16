#ifndef ERPROC_H
#define ERPROC_H

#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1000
#define BUFFER_SIZE_NAME 50

typedef struct {
    char username[BUFFER_SIZE_NAME];
    char message[BUFFER_SIZE];
} DataSend;

typedef struct {
    char username[BUFFER_SIZE_NAME];
    long int port;
    struct in_addr server_ip;
    unsigned char verbose;
} ChatConfig;


int Socket(const int domain, const int type, const int protocol);
void Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void Pthread_create(pthread_t *pthread, const pthread_attr_t * attr, void *(*func)(void *), void *data);


void log_message(int flag, const char *format, ...);
void print_help(char *app_name);
ChatConfig parse_argumets(int argc, char **argv);


void* listening_port(void *arg);
void* broadcast_send_data(void *arg);

#endif