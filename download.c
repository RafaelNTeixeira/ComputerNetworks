#include "src/parser.c"
#include "src/clientTCP.c"
#include "src/getip.c"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("ERROR MAIN START // URL should be as follows: ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    url inputURL;
    defaultParser(&inputURL);
    parse(&inputURL, argv[1]);

    printf("Username: %s\n", inputURL.user);
	printf("Password: %s\n", inputURL.password);
	printf("Host: %s\n", inputURL.host);
	printf("Path: %s\n", inputURL.path);
	printf("Filename: %s\n", inputURL.filename);

    char IPAddress[20] = "";

    if (getIP(inputURL.host, IPAddress) != 0) {
        printf("getIP error on main\n");
        exit(-1);
    }

    if (connectionDownload(&inputURL, IPAddress) != 0) {
        printf("connectionDownload error on main \n");
        exit(-1);
    }
    
    return 0;
}