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
typedef char buildUrl[MAX_LENGTH];

#define START 0
#define USERNAME 1
#define PASSWORD 2
#define HOST 3
#define PATH 4

#define SERVICE_READY 220
#define USER_NAME_OK 331
#define LOGGED_IN 230
#define ENTER_PASSIVE 227
#define FILE_OK 150
#define DOWNLOAD 226

typedef struct URL{
    buildUrl user;
    buildUrl password;
    buildUrl path;
    buildUrl host;
    buildUrl filename;
    int port;
} url;


void reverseString(char *str); // revstr n funciona built in ????
void defaultParser(url *url); //definir os default;
void parserURL(url *url, const char *input); //str para tirar os componentes do url

