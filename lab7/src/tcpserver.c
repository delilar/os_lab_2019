#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 10050
#define DEFAULT_BUFSIZE 100

int main(int argc, char *argv[]) {
    int port = argc > 1 ? atoi(argv[1]) : DEFAULT_PORT;
    int bufsize = argc > 2 ? atoi(argv[2]) : DEFAULT_BUFSIZE;
    char buf[bufsize];

    int lfd, cfd, nread;
    struct sockaddr_in servaddr, cliaddr;
    const size_t kSize = sizeof(struct sockaddr_in);

    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, kSize);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *)&servaddr, kSize) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(lfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    while (1) {
        unsigned int clilen = kSize;

        if ((cfd = accept(lfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept");
            exit(1);
        }
        printf("connection established\n");

        while ((nread = read(cfd, buf, bufsize)) > 0) {
            write(1, &buf, nread);
        }

        if (nread == -1) {
            perror("read");
            exit(1);
        }
        close(cfd);
    }
}
