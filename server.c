#include<stdio.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define SERV_TCP_PORT 15038
#define MAX 60

int i, j, tem;
char buff[4096], t[MAX];;
FILE *f1;
int main(int arg, char *argv[])
{
    int sockfd, newsockfd, clength;
    struct sockaddr_in serv_addr,cli_addr;
    char t[MAX], str[MAX];
    strncpy(t, "exit", MAX);
    t[MAX - 1] = '\0';
    sockfd=socket(AF_INET, SOCK_STREAM,0);

    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(SERV_TCP_PORT);
    printf("\nBinded");
    bind(sockfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("\nListening...");
    listen(sockfd, 5);
    clength=sizeof(cli_addr);
    newsockfd=accept(sockfd,(struct sockaddr*) &cli_addr,&clength);
    close(sockfd);
    read(newsockfd, &str, sizeof(str));
    printf("\nClient message\n File Name : %s\n", str);
    f1=fopen(str, "r");
    if (f1 == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    while (fgets(buff, sizeof(buff), f1) != NULL) {
        write(newsockfd, buff, strlen(buff));
    }
    if (f1 != NULL) {
        fclose(f1);
    }
    printf("\nFile Transferred\n");
    return 0;
}