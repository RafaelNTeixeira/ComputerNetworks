#include "download.h"

int parseArguments(char *url, struct parseArguments *pa){
    printf("Parsing command line arguments...\n");

    regex_t regex;
    regcomp(&regex, BAR, 0);
    if (regexec(&regex, pa, 0, NULL, 0)) return -1;

    regcomp(&regex, AT, 0);
    if (regexec(&regex, pa, 0, NULL, 0) != 0) { //ftp://<host>/<url-path>
        
        sscanf(pa, URL_WITHOUT_U_AND_P, pa->host);
        strcpy(pa->user, "anonymous");
        strcpy(pa->password, "password");

    } else { // ftp://[<user>:<password>@]<host>/<url-path>

        sscanf(pa, URL_WITH_U_AND_P, pa->host);
        sscanf(pa, USER_URL, pa->user);
        sscanf(pa, PASS_URL, pa->password);
    }

    sscanf(pa, RESOURCE_URL, pa->path);
    strcpy(pa->file, strrchr(pa, '/') + 1);

    return 0;
}

int getIpAddress(char *ip, char *host){
    if ((h = gethostbyname(host)) == NULL) {
        printf("gethostbyname() ERROR!");
        return -1;
    }
    strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
    return 0;
}

int createAndConnectSocket(char *ip, int port){
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

int stateMachine(int socket, char *buf){
    char byte;
    State state = START;
    int i = 0;
    memset(buf, 0, MAX_LENGTH);

    while(state != END){
        read(socket, %byte, 1);
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

    int code = atoi(buf);
    return code;
}

int authenticate(int socket, char *user, char *pass){
    char userCom[5+strlen(user)+1]; // Não sei porque se adiciona 5 e 1
    char passCom[5+strlen(pass)+1]; 
    char res[MAX_LENGTH];

    write(socket, userCom, strlen(userCom));
    if (stateMachine(socket, res) != READY4PASS){
        printf("Unknown user!");
        return -1;
    }

    write(socket, passCom, strlen(passCom));
    return stateMachine(socket, res);
}


int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    } 

    struct parseArguments pa;
    memset(&pa, 0, sizeof(pa));

    if(parseArguments(argv[1], &pa) != 0){
        printf("Parse error!");
        exit(-1);
    }

    if(getIpAddress(pa.ip, pa.host) != 0){
        printf("getIpAddress() error!");
        exit(-1);
    }

    printf("User: %s\n", pa.user);
    printf("Password: %s\n", pa.password);
    printf("Host name: %s\n", pa.host);
    printf("File path: %s\n", pa.path);
    printf("File name: %s\n", pa.file);
    printf("IP Address: %s\n", pa.ip);

    char answer[MAX_LENGTH];
    int socketA = createAndConnectSocket(pa.ip, FTP_PORT);
    if (socketA < 0) {
        printf("Socket creation failed!");
        exit(-1);
    }

    char res[MAX_LENGTH];

    if (stateMachine(socketA, res) != READY4AUTH) {
        printf("Server not ready to authenticate.");
        exit(-1);
    }

    if (authenticate(socketA, pa.user, pa.path) != LOGINSUCCESS){
        printf("Authentication failed!");
        exit(-1);
    }


}