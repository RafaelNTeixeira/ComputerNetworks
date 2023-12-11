/**
 * Example code for getting the IP address from hostname.
 * tidy up includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>



int getIP(char *hostname, char* IPaddress){
    struct hostent *h;
    
    if ((h = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    /* Retrieve IP*/
    strcpy(IPaddress, inet_ntoa(*((struct in_addr *) h->h_addr)));

    printf("Hostname  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

    return 0;
}


