#include "../include/parser.h"
#include "string.h"

void defaultParser(url* url) {
    memset(url->user, 0, MAX_LENGTH);
    memset(url->password, 0, MAX_LENGTH);
    memset(url->host, 0, MAX_LENGTH);
    memset(url->path, 0, MAX_LENGTH);
    memset(url->filename, 0, MAX_LENGTH);
    url->port = 21; // default de ftp control port -> Permite a conexão com o server uma vez que abre automaticamente
}

void setDefaultUserInfo(url *url) {
    memcpy(url->host, url->user, MAX_LENGTH);
    memset(url->user, 0, MAX_LENGTH);
    memcpy(url->user, "anonymous", 9);
    memcpy(url->password, "anonymous", 9);
}

void extract_filename(url *url) {
    int it = 0;
    for (int i = strlen(url->path) - 1; i >= 0 ; i--) {
        if (url->path[i] == '/') {
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

void parse(url *url, const char *input) {
    printf("Parsing...\n");

    int state = STATE_FTP;
    int infIndex = 0;
    int inputSize = strlen(input);
    char urlStart[] = "ftp://";

    for (int i = 0; i < inputSize; i++) {
        switch (state) {
            case STATE_FTP:
            if (strncmp(input, "ftp://", 6) != 0) {
                printf("Error: URL does not start with ftp://\n");
                return;
            }
            state = STATE_USER;
            i += 5; // Skip the "ftp://" part
            break;
            case STATE_USER:
                if (input[i] == ':') {
                    state = STATE_PASSWORD;
                    infIndex = 0;
                } else if (input[i] == '/') { // No Username or Password
                    setDefaultUserInfo(url);                  
                    state = STATE_PATH; // Skip to path
                    infIndex = 0; 
                } else {
                    url->user[infIndex] = input[i];
                    infIndex++;
                }
                break;
            case STATE_PASSWORD:
                if (input[i] == '@') {
                    state = STATE_HOST;
                    infIndex = 0;
                } else {
                    url->password[infIndex] = input[i];
                    infIndex++;
                }
                break;
            case STATE_HOST:
                if (input[i] == '/') {
                    state = STATE_PATH;
                    infIndex = 0;
                } else {
                    url->host[infIndex] = input[i];
                    infIndex++;
                }
                break;
            case STATE_PATH: 
                url->path[infIndex] = input[i];
                infIndex++;
                break;
            default:
                break;
        }
    }

    if (state == STATE_USER || state == STATE_PASSWORD) {
        printf("Error: Incomplete URL\n");
        return;
    }

    url->path[infIndex] = '\0'; // Null terminate the path

    extract_filename(url);
}




