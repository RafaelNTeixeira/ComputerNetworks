/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int createSocket(char *serverAddress,int port){ //sv_add = ip macro
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverAddress);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
}

int getLastLineStatusCode(const char *buf) {
    int a = 0;
    const char *start = buf;
    
    while (*buf != '\0') {
        if (*buf == '\n') {
            a = atoi(start);
            start = buf + 1; 
        }
        buf++;
    }
    if (start != buf) {
        a = atoi(start);
    }

    return a;
}

int getPortNumber(char *buf) {
    int num[5] = {0};
    int i = 0;
    char *pt;
    pt = strtok(buf, ",");
    pt = strtok(NULL, ",");
    while (pt != NULL) {
        int a = atoi(pt);
        num[i] = a;
        pt = strtok(NULL, ",");
        i++;
    }
    return (num[3] * 256 + num[4]);
}

int downloadFileFromDataConnection(int dataSocket, FILE *fileptr) {
    char buffer[500];
    size_t bytesRead;

    while ((bytesRead = read(dataSocket, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytesRead, fileptr);
    }

    return bytesRead;  
}

int handleEnterPassive(int sockfd, char* buf, char* IPAddress, char* retrvPath) {
    int port = getPortNumber(buf);
    int sockfd2 = createSocket(IPAddress, port);

    if (sockfd2 == -1) {
        fprintf(stderr, "Error creating data connection\n");
        return -1;
    }

    write(sockfd, retrvPath, strlen(retrvPath));
    printf("Data connection created\n");
    return sockfd2;
}

void handleDownload(int sockfd2, FILE* fileptr) {
    char buf2[1024];
    ssize_t bytes2;
    while (1) {
        memset(buf2, 0, sizeof(buf2));
        bytes2 = read(sockfd2, buf2, sizeof(buf2));
        if (bytes2 != -1 && bytes2 != 0) {
            for (int i = 0; i < bytes2; i++) {
                printf("%c", buf2[i]);
                fputc(buf2[i], fileptr);
            }
        } else {
            break;
        }
    }
}

int connectionDownload(url *url, char *IPAddress) {
    int sockfd = createSocket(IPAddress, url->port);
    if (sockfd == -1) {
        fprintf(stderr, "Error creating socket\n");
        return -1;
    }

    char userLogin[263];
    snprintf(userLogin, sizeof(userLogin), "user %s\r\n", url->user);

    char passwdLogin[263];
    snprintf(passwdLogin, sizeof(passwdLogin), "pass %s\r\n", url->password);

    char retrvPath[263];
    snprintf(retrvPath, sizeof(retrvPath), "retr %s\r\n", url->path);

    int sockfd2 = 0;
    FILE *fileptr = NULL;
    int port = 0;
    int download = 0;
    int STOP = 0;
    int visited = 0;
    size_t bytes, bytes2;
    char buf[500] = {0};
    char buf2[500] = {0};

    while (!STOP) {
        memset(buf, 0, sizeof(buf));
        bytes = read(sockfd, buf, sizeof(buf));

        if (download) {
            memset(buf2, 0, sizeof(buf2));
            bytes2 = read(sockfd2, buf2, sizeof(buf2));
            if (bytes2 != -1 && bytes2 != 0) {
                for (int i = 0; i < bytes2; i++) {
                    printf("%c", buf2[i]);
                    fputc(buf2[i], fileptr);
                }
            }
        }

        if (bytes == -1 || bytes == 0) {
            continue;
        }

        printf("\n%s\n", buf);
        int statusCode = getLastLineStatusCode(buf);

        switch (statusCode) {
            case SERVICE_READY: 
                if (!visited) {
                    visited = 1;
                    write(sockfd, userLogin, strlen(userLogin));
                    printf("User login sent\n");
                }
                break;
            case USER_NAME_OK: 
                write(sockfd, passwdLogin, strlen(passwdLogin));
                printf("Password sent\n");
                break;
            case LOGGED_IN: 
                write(sockfd, "pasv\r\n", 6);
                printf("Entering passive mode\n");
                break;
            case ENTER_PASSIVE: 
                sockfd2 = handleEnterPassive(sockfd, buf, IPAddress, retrvPath);
                if (sockfd2 == -1) {
                    return -1;
                }
                break; 
            case FILE_OK: 
                fileptr = fopen(url->filename, "w");
                download = 1;
                printf("File opened and ready for download\n");
                break;
            case DOWNLOAD: 
                handleDownload(sockfd2, fileptr);
                download = 0;
                STOP = 1;
                printf("Download completed\n");
                break;
            default:
                fprintf(stderr, "Received unexpected status code: %d\n", statusCode);
                return -1;
        }
    }

    fclose(fileptr);

    if (close(sockfd2) < 0) {
        perror("close()");
        return -1;
    }

    if (close(sockfd) < 0) {
        perror("close()");
        return -1;
    }

    return 0;
}
