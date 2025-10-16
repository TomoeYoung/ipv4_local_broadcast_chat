#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "erproc.h"

#define VERBOSE config.verbose
#define IP_SERVER config.server_ip
#define NAME config.username
#define PORT config.port

int main(int argc, char **argv) {
    ChatConfig config = parse_argumets(argc, argv);

    log_message(VERBOSE, "\n-------------------------------\n");
    log_message(VERBOSE, "Name user: %s\n", NAME);
    log_message(VERBOSE, "Port listen: %d\n", PORT);
    log_message(VERBOSE, "IPv4 server listen: %s\n", inet_ntoa(IP_SERVER));
    log_message(VERBOSE, "-------------------------------\n");

    pthread_t pthreads[2];

    log_message(VERBOSE, "Try run pthread for listening...\n");
    Pthread_create(&pthreads[0], NULL, listening_port, &config);
    log_message(VERBOSE, "Pthread for listening run successful.\n");

    log_message(VERBOSE, "Try run pthread for sendto...\n");
    Pthread_create(&pthreads[1], NULL, broadcast_send_data, &config);
    log_message(VERBOSE, "Pthread for sendto run successful.\n");

    pthread_join(pthreads[0], NULL);
    pthread_join(pthreads[1], NULL);

    return 0;
}