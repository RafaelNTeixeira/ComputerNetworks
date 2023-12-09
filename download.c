#include "download.h"

int parseArguments(char *url, struct parseArguments *pa) {
    printf("Parsing command line arguments...\n");

    // Skip the "ftp://" part
    char *p = strstr(url, "ftp://");
    if (p == NULL) return -1;
    p += 6;  // Skip "ftp://"

    // Check if the URL contains a username and password 
    // ftp://[<user>:<password>@]<host>/<url-path>
    char *at = strchr(p, '@');
    if (at != NULL) {
        // The URL contains a username and password
        *at = '\0';  // Split the string into two parts

        // Get the username and password
        char *colon = strchr(p, ':');
        if (colon == NULL) return -1;
        *colon = '\0';  // Split the string into two parts

        strcpy(pa->user, p);
        strcpy(pa->password, colon + 1);

        p = at + 1;  // Skip the "@"
    } else {
        //ftp://<host>/<url-path>
        strcpy(pa->user, "anonymous");
        strcpy(pa->password, "anonymous@");
    }

    // Get the host
    char *slash = strchr(p, '/');
    if (slash == NULL) return -1;
    *slash = '\0';  // Split the string into two parts
    strcpy(pa->host, p);

    // Get the path and file
    strcpy(pa->path, slash + 1);
    strcpy(pa->file, strrchr(pa->path, '/') + 1);

    return 0;
}

int getIpAddress(char *ip, char *host) {
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL) {
        printf("gethostbyname() ERROR!");
        return -1;
    }
    strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
    return 0;
}

int createAndConnectSocket(char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    /*connect to the server*/
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }

    return sockfd;
}

int stateMachine(const int socket, char *buf) {
    char byte = '\0';
    State state = START;
    int i = 0;
    memset(buf, 0, MAX_LENGTH);

    while(state != END){
        read(socket, &byte, 1);
        printf("byte: %s\n", &byte);
        switch (state) {
            case START:
                if (byte == ' ') state = SPACE;
                else if (byte == '-') state = TRACE;
                else if (byte == '\n') state = END;
                else buf[i++] = byte; 
                break;
            
            case SPACE:
                if(byte == '\n') state = END;
                else buf[i++] = byte;
                break;
            
            case TRACE:
                if(byte == '\n'){
                    memset(buf, 0, MAX_LENGTH);
                    state = START;
                    i = 0;
                }
                else buf[i++] = byte;
                break;

            case END:
                break;

            default:
                break;
        }
    }

    printf("Buf: %s\n", buf);
    int code = atoi(buf);
    printf("code: %d\n", code);
    return code;
}

int authenticate(int socket, char *user, char *pass) {
    char userCom[5+strlen(user)+1]; 
    char passCom[5+strlen(pass)+1]; 
    char res[MAX_LENGTH];

    snprintf(userCom, sizeof(userCom), "USER %s\n", user);
    snprintf(passCom, sizeof(passCom), "PASS %s\n", pass);

    printf("userCom: %s\n", userCom);
    printf("passCom: %s\n", passCom);

    write(socket, userCom, strlen(userCom));

    if (stateMachine(socket, res) != READY4PASS){
        printf("Unknown user!");
        return -1;
    }

    write(socket, passCom, strlen(passCom));
    return stateMachine(socket, res);
}

int switchPassiveMode(int socket, char *ip, int *portFT) {
    char res[MAX_LENGTH];

    write(socket, "pasv\n", 5);
    if(stateMachine(socket, res) != PASSIVE){
        printf("Passsive Mode Error!");
        return -1;
    }

    int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(res, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    *portFT = port1 * 256 + port2;

    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    return 0;
}

int resqPath(int socket, char *path) {
    char res[MAX_LENGTH];
    char fileCom[MAX_LENGTH];

    snprintf(fileCom, sizeof(fileCom), "path %s\n", path);

    write(socket, fileCom, sizeof(fileCom));

    if(stateMachine(socket, res) != READY4TRANSFER) {
        return -1;
    }

    return 0;
}

int getFile(int socketA, int socketB, char *fileName){
    FILE *file = fopen(fileName, "wb");

    if (file == NULL){
        printf("Error opening or creating file");
        return -1;
    }

    char buf[MAX_LENGTH];
    ssize_t bytes;

    do {
        bytes = read(socketB, buf, MAX_LENGTH);
        if (bytes < 0) {
            perror("Error reading from socket");
            fclose(file);
            return -1;
        }

        if (fwrite(buf, bytes, 1, file) < 0) {
            perror("Error writing to file");
            fclose(file);
            return -1;
        }
    } while (bytes > 0);

    fclose(file);

    if(stateMachine(socketA, buf) != TRANSFER_COMPLETE){
        return -1;
    }
    return 0;
}


int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    } 

    struct parseArguments pa;
    memset(&pa, 0, sizeof(pa));

    if (parseArguments(argv[1], &pa) != 0) {
        printf("Parse error!");
        exit(-1);
    }

    if (getIpAddress(pa.ip, pa.host) != 0){
        printf("getIpAddress() error!");
        exit(-1);
    }

    printf("User: %s\n", pa.user);
    printf("Password: %s\n", pa.password);
    printf("Host name: %s\n", pa.host);
    printf("File path: %s\n", pa.path);
    printf("File name: %s\n", pa.file);
    printf("IP Address: %s\n", pa.ip);

    int socketA = createAndConnectSocket(pa.ip, FTP_PORT);
    if (socketA < 0) {
        printf("Socket A creation failed!");
        exit(-1);
    }

    char res[MAX_LENGTH];

    if (stateMachine(socketA, res) != READY4AUTH) {
        printf("Server not ready to authenticate.");
        exit(-1);
    }

    printf("here\n");
    if (authenticate(socketA, pa.user, pa.password) != LOGINSUCCESS){
        printf("Authentication failed!");
        exit(-1);
    }

    int portFT;
    char serverIP[MAX_LENGTH] ;
    if (switchPassiveMode(socketA, serverIP, &portFT) != 0){
        printf("Passive mode failed\n");
        exit(-1);
    }

    int socketB = createAndConnectSocket(pa.ip, FTP_PORT);
    if (socketA < 0) {
        printf("Socket B creation failed!\n");
        exit(-1);
    }

    if(resqPath(socketA, pa.path) != 0){
        printf("Unknown path\n");
        exit(-1);
    }

    if(getFile(socketA, socketB, pa.file) != 0){
        printf("Error transfering file!\n");
        exit(-1);
    }

    write(socketA, "quit\n", 5);

    char answer[MAX_LENGTH];

    if(stateMachine(socketA, answer) != GOODBYE){
        printf("GoodBye Error!\n");
        exit(-1);
    }

    if (close(socketA) < 0) {
        perror("close()");
        exit(-1);
    }

    if (close(socketB) < 0) {
        perror("close()");
        exit(-1);
    }

    return 0;
}