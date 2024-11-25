#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 20001
#define DEFAULT_BUFSIZE 1024

int main(int argc, char *argv[]) {
    int port = argc > 1 ? atoi(argv[1]) : DEFAULT_PORT;
    int bufsize = argc > 2 ? atoi(argv[2]) : DEFAULT_BUFSIZE;

    char mesg[bufsize], ipadr[16];
    int sockfd, n;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind problem");
        exit(1);
    }

    printf("SERVER starts...\n");

    while (1) {
        unsigned int len = sizeof(cliaddr);

        if ((n = recvfrom(sockfd, mesg, bufsize, 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("recvfrom problem");
            exit(1);
        }
        mesg[n] = 0;

        printf("REQUEST %s FROM %s : %d\n", mesg,
               inet_ntop(AF_INET, &cliaddr.sin_addr, ipadr, 16),
               ntohs(cliaddr.sin_port));

        if (sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len) < 0) {
            perror("sendto problem");
            exit(1);
        }
    }
}
