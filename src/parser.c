#include "../include/parser.h"
#include "string.h"

void defaultParser(url* url) {
    memset(url->user,0,MAX_LENGTH);
    memset(url->password,0,MAX_LENGTH);
    memset(url->host,0,MAX_LENGTH);
    memset(url->path,0,MAX_LENGTH);
    memset(url->filename,0,MAX_LENGTH);
    url->port = 21; // default de ftp control port -> Permite a conexão com o server uma vez que abre automaticamente
}

void setDefaultUserInfo(url *url) {
    memcpy(url->host, url->user, MAX_LENGTH);
    memset(url->user, 0, MAX_LENGTH);
    memcpy(url->user, "anonymous", 9);
    memcpy(url->password, "anonymous", 9);
}

void parse(url *url, const char *input) {
    printf("Parsing testing \n");

    int state = STATE_FTP;
    int aux = 0;
    int inputSize = strlen(input);
    char urlStart[] = "ftp://";

    for (int i = 0; i < inputSize; i++) {
        switch(state) {
            case STATE_FTP:
                if (input[i] != urlStart[i]) {
                    printf("Error: URL does not start with ftp://\n");
                    return;
                } else if (i == 5) {
                    state = STATE_USER;
                }
                break;
            case STATE_USER:
                if(input[i] == ':') {
                    state = STATE_PASSWORD;
                    aux = 0;
                } else if(input[i] == '/') { // No Username or Password
                    setDefaultUserInfo(url);                  
                    state = STATE_PATH;   
                    aux = 0; 
                } else {
                    url->user[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_PASSWORD:
                if(input[i] == '@') {
                    state = STATE_HOST;
                    aux = 0;
                } else {
                    url->password[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_HOST:
                if(input[i] == '/') {
                    state = STATE_PATH;
                    aux = 0;
                } else {
                    url->host[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_PATH: 
                url->path[aux] = input[i];
                aux++;
                break;
            default:
                break;
        }
    }

    if (state == STATE_USER || state == STATE_PASSWORD) {
        printf("Error: Incomplete URL\n");
        return;
    }

    url->path[aux] = '\0'; // Null terminate the path

    int it = 0;
    for (int i = strlen(url->path) - 1; i >= 0 ; i--) {
        if(url->path[i] == '/') {
            break;
        }
        url->filename[it++] = url->path[i];
    }
    url->filename[it] = '\0'; // Null terminate the string

    // Reverse the filename
    for (int start = 0, end = it - 1; start < end; start++, end--) {
        char temp = url->filename[start];
        url->filename[start] = url->filename[end];
        url->filename[end] = temp;
    }
}




