#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 15038
#define MAX 60

int main() {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char send[MAX], recvline[MAX];

    // Replace "127.0.0.1" with your server's IP address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Enter the source file name: ");
    if (scanf(" %s", send) != 1) {
        fprintf(stderr, "Error reading file name\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, send, MAX) == -1) {
        perror("Write to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while ((n = read(sockfd, recvline, MAX)) > 0) {
        printf("%.*s", n, recvline);
    }

    if (n == -1) {
        perror("Read from server failed");
    }

    close(sockfd);
    return 0;
}
