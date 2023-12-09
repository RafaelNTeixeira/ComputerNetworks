#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <regex.h>

#include <string.h>

#define MAX_LENGTH  300
#define FTP_PORT    21

#define READY4AUTH           220
#define READY4PASS           331
#define LOGINSUCCESS         230
#define PASSIVE              227
#define READY4TRANSFER       150
#define TRANSFER_COMPLETE    226
#define GOODBYE              221

/* Regular expresions */
#define AT                      "@"
#define BAR                     "/"
#define URL_WITHOUT_U_AND_P     "%*[^/]//%[^/]"
#define URL_WITH_U_AND_P        "%*[^/]//%*[^@]@%[^/]"
#define RESOURCE_URL            "%*[^/]//%*[^/]/%s"
#define USER_URL                "%*[^/]//%[^:/]"
#define PASS_URL                "%*[^/]//%*[^:]:%[^@\n$]"
#define PASSIVE_REGEX           "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]"


struct parseArguments {
    char host[MAX_LENGTH];      // 'ftp.up.pt'
    char path[MAX_LENGTH];
    char file[MAX_LENGTH];
    char user[MAX_LENGTH];      // 'username'
    char password[MAX_LENGTH];  // 'password'
    char ip[MAX_LENGTH];
};

typedef enum {
    START,
    SPACE,
    TRACE,
    END  
}State;

int parseArguments(char *input, struct parseArguments *pa);
int getIpAddress(char *ip, char *host);
int createAndConnectSocket(char *ip, int port);
int stateMachine(int socket, char *buf);
int authenticate(int socket, char *user, char *pass);
int switchPassiveMode(int socket, char *ip, int *portFT);
int resqPath(int socket, char *path);
int getFile(int socketA, int socketB, char *fileName);
