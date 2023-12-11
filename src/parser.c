#include "../include/parser.h"
#include "string.h"

void reverseString(char *str) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;

    while (start < end) {

        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;


        start++;
        end--;
    }
}
void defaultParser(url* url){
    memset(url->user,0,MAX_LENGTH);
    memset(url->password,0,MAX_LENGTH);
    memset(url->host,0,MAX_LENGTH);
    memset(url->path,0,MAX_LENGTH);
    memset(url->filename,0,MAX_LENGTH);
    url->port = 21; // default de ftp control port -> permite a conexao com o server uma vez que abre automaticamente
    // o transfer do file data -> WinZip
}

#define STATE_FTP 0
#define STATE_USER 1
#define STATE_PASSWORD 2
#define STATE_HOST 3
#define STATE_PATH 4

void parse(url *url, const char *input) {
    printf("Parsing testing \n");

    int state = STATE_FTP;
    int aux = 0;
    int input_size = strlen(input);
    char urlStart[] = "ftp://";

    for(int i = 0; i < input_size; i++) {
        switch(state) {
            case STATE_FTP: // ftp part
                if (input[i] != urlStart[i]) {
                    printf("Error: URL does not start with ftp://\n");
                    return;
                } else if (i == 5) {
                    state = STATE_USER;
                }
                break;
            case STATE_USER: // username
                if(input[i] == ':') {
                    state = STATE_PASSWORD;
                    aux = 0;
                } else if(input[i] == '/') { // no username or password
                    memcpy(url->host, url->user, MAX_LENGTH);
                    memset(url->user, 0, MAX_LENGTH);
                    memcpy(url->user, "anonymous", 9);
                    memcpy(url->password, "anonymous", 9);                   
                    state = STATE_PATH;   
                    aux = 0; 
                } else {
                    url->user[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_PASSWORD: // password
                if(input[i] == '@') {
                    state = STATE_HOST;
                    aux = 0;
                } else {
                    url->password[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_HOST: // host
                if(input[i] == '/') {
                    state = STATE_PATH;
                    aux = 0;
                } else {
                    url->host[aux] = input[i];
                    aux++;
                }
                break;
            case STATE_PATH: // path
                url->path[aux] = input[i];
                aux++;
                break;
        }
    }
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




