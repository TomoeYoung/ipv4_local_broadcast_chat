#include "erproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <arpa/inet.h>

int Socket(const int domain, const int type, const int protocol) {
    const int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("Error: create server-socket.\n");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        fprintf(stderr, "Error: setsockopt failed.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) == -1) {
        perror("Error: bind failed.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void Pthread_create(pthread_t *pthread, const pthread_attr_t * attr, void *(*func)(void *), void *data) {
    if (pthread_create(pthread, attr, func, data) != 0) {
        fprintf(stderr, "Error: create pthread.\n");
        exit(EXIT_FAILURE);
    }
}

void log_message(int flag, const char *format, ...) {
    if (flag) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void print_help(char *app_name) {
    printf("Usage: %s [options]\n\n", app_name);
    printf("Options with argument: \n");
    printf("  --address   ADDRESS IP (example: 192.168.1.0)\n");
    printf("  --port      PORT (1-65535)\n");
    printf("\nOptions without argument: \n");
    printf("  --verbose   OUTPUT DEBUG INFORMATION\n");
    printf("  --help      INFORMATION ABOUT OPTIONS\n");
    printf("\n-----AVAILABLE OPTIONS-----\n");
}

static struct option long_option[] = {
    {"help", no_argument, 0, 'h'},
    {"address", required_argument, 0, 'a'},
    {"port", required_argument, 0, 'p'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

ChatConfig parse_argumets(int argc, char **argv) {
    ChatConfig config = {
        .username = "",
        .port = -1,
        .verbose = 0
    };

    if (argc == 1) {
        printf("%s: missing file operand\n", argv[0]);
        printf("Try '%s --help' for more information.\n", argv[0]);
        exit(0);
    }

    char ip[INET_ADDRSTRLEN] = "";
    char *endptr;
    int option_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "a:p:vh", long_option, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'a':
                strncpy(ip, optarg, INET_ADDRSTRLEN - 1);
                if (inet_pton(AF_INET, ip, &(config.server_ip)) != 1) {
                    fprintf(stderr, "ERROR: invalid IPv4 address: %s\n", ip);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                config.port = strtol(optarg, &endptr, 10);
                if (errno == ERANGE || endptr == optarg) {
                    fprintf(stderr, "ERROR: failed to convert port\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                config.verbose = 1;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", opt);
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    if (strlen(ip) == 0) {
        fprintf(stderr, "Error: option [--address=ip] is required.\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (config.port <= 0 || config.port > 65535) {
        fprintf(stderr, "Error: option [--port=port] is required and (0 <= port <= 65535)\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Enter your nickname:\n");
    scanf("%s", config.username);

    return config;
}

void* listening_port(void *arg) {
    int sockfd;
    struct sockaddr_in serveraddr, cliaddr;
    socklen_t len;
    ChatConfig *config = (ChatConfig *)arg;

    log_message(config->verbose,"\nCreate SOCKET...\n");

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    int yes = 1;
    Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef SO_REUSEPORT
    Setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config->port);
    serveraddr.sin_addr = config->server_ip;

    Bind(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));

    len = sizeof(cliaddr);
    printf("\nServer is running listening on %s:%ld...\n", inet_ntoa(config->server_ip), config->port);

    for (;;) {
        DataSend msg_rec;
        size_t n = recvfrom(sockfd, &msg_rec, sizeof(msg_rec), 0, (struct sockaddr*)&cliaddr, &len);
        if (n > 0) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cliaddr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message at %s from %s:%d -> %s\n",
                   msg_rec.username, client_ip, htons(cliaddr.sin_port), msg_rec.message);
        }
    }
}

void *broadcast_send_data(void *arg) {
    int sockfd;
    char buffer_message[BUFFER_SIZE];
    struct sockaddr_in serveraddr;
    ChatConfig *config = (ChatConfig *)arg;
    int broadcast_enable = 1;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    Setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config->port);
    serveraddr.sin_addr.s_addr = INADDR_BROADCAST;

    printf("Enter messages up to 1000 bytes to send 255.255.255.255:%ld...\n", config->port);
    printf("Press Ctrl+C to stop the server.\n");

    for (;;) {
        scanf("%s", buffer_message);
        buffer_message[strcspn(buffer_message, "\n")] = '\0';

        DataSend msg;
        strncpy(msg.username, config->username, sizeof(msg.username) - 1);
        strncpy(msg.message, buffer_message, sizeof(msg.message) - 1);

        ssize_t n = sendto(sockfd, &msg, sizeof(msg), 0,
                           (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
        if (n < 0) perror("Sendto failed\n");
        else log_message(config->verbose, "Message sent successfully (%zd bytes)\n", n);
    }
}
