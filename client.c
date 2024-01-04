#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

// Function to extract hostname and path from the URL
int parse_url(const char *url, char *hostname, char *path) {
    if (sscanf(url, "http://%[^/]/%s", hostname, path) != 2) {
        fprintf(stderr, "Invalid URL format\n");
        return 1;
    }
    return 0;
}

// Function to download a file from an HTTP server
int download_file(const char *url, const char *output_file) {
    char hostname[256];
    char path[256];

    // Parse URL to extract hostname and path
    if (parse_url(url, hostname, path) != 0) {
        return 1;
    }

    // Resolve hostname to IP address
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        perror("Error resolving hostname");
        return 1;
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Set up server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

    // Send HTTP GET request
    dprintf(sockfd, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);

    // Open the output file
    FILE *file = fopen(output_file, "wb");
    if (file == NULL) {
        perror("Error opening output file");
        close(sockfd);
        return 1;
    }

    // Receive and save the file data
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(sockfd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytesRead, file);
    }

    // Clean up
    fclose(file);
    close(sockfd);

    printf("Download complete. File saved as %s\n", output_file);

    return 0;
}

int main() {
    // URL of the file to download
    const char *url = "http://example.com/file.txt";

    // Output file name
    const char *output_file = "downloaded_file.txt";

    // Download the file
    if (download_file(url, output_file) != 0) {
        fprintf(stderr, "Error downloading file\n");
        return 1;
    }

    return 0;
}