#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define MAX_LENGTH 256

#define STATE_FTP 0
#define STATE_USER 1
#define STATE_PASSWORD 2
#define STATE_HOST 3
#define STATE_PATH 4

#define SERVICE_READY 220
#define USER_NAME_OK 331
#define LOGGED_IN 230
#define ENTER_PASSIVE 227
#define FILE_OK 150
#define DOWNLOAD 226

typedef struct URL{
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char path[MAX_LENGTH];
    char host[MAX_LENGTH];
    char filename[MAX_LENGTH];
    int port;
} url;

void defaultParser(url *url);
void setDefaultUserInfo(url *url);
void parse(url *url, const char *input);

