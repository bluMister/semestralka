#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define MAX_VLAKIEN 100

// Struktúra pre uchovanie informácií o vlákne
typedef struct {
    pthread_t id;
    int cas;
} VlaknoInfo;

//download logger
void logger(char *log, bool write) {

    if (write) {
        time_t t;
        struct tm *current_time;
        char buffer[100];

        time(&t);
        current_time = localtime(&t);

        // Format the date and time
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", current_time);

        strcat(buffer, log);

        // Open a file for writing
        FILE *file = fopen("downHistory.txt", "w");

        if (file == NULL) {
            printf("Error opening file!\n");
        }

        // Write the formatted date and time to the file
        fprintf(file, "%s\n", buffer);

        // Close the file
        fclose(file);
    } else {
        FILE *file = fopen("downHistory.txt", "r");

        if (file == NULL) {
            printf("Error opening file for reading!\n");
        }

        // Read and display the contents of the file
        printf("Contents of 'output.txt':\n");

        char line[100];
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }

        // Close the file
        fclose(file);
    }
}

// Function to extract hostname and path from the URL
int parse_url(const char *url, char *hostname, char *path, int *is_https) {
    if (sscanf(url, "http://%[^/]/%s", hostname, path) == 2) {
        *is_https = 0;
        return 0;
    } else if (sscanf(url, "https://%[^/]/%s", hostname, path) == 2) {
        *is_https = 1;
        return 0;
    }

    fprintf(stderr, "Invalid URL format\n");
    return 1;
}

//function to read download folder path from file
char *loadDownFolderPath() {
    FILE *file = fopen("downFolderPath.txt", "r");
    if (file == NULL) {
        perror("Error opening path file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *) malloc(size + 1);
    if (content == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    fread(content, 1, size, file);
    content[size] = '\0'; // Null-terminate the string

    fclose(file);
    content[strcspn(content, "\n")] = 0;
    return content;
}

// Function to extract filename from the URL
const char *extract_filename(const char *url) {
    const char *last_slash = strrchr(url, '/');
    return (last_slash != NULL) ? (last_slash + 1) : url;
}

// Function to set up SSL context
SSL_CTX *create_ssl_context() {
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    return ctx;
}

// Function to download a file from an HTTP or HTTPS server
int download_file(const char *url, int timer) {
    char hostname[256];
    char path[256];
    int is_https;

    // Parse URL to extract hostname and path
    if (parse_url(url, hostname, path, &is_https) != 0) {
        return 1;
    }

    //check for timer and wiat if necessary
    if (timer > 0) {
        sleep(timer * 60);
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
    server_addr.sin_port = htons(is_https ? 443 : 80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

    // If using HTTPS, set up SSL
    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;
    if (is_https) {
        SSL_library_init();
        ssl_ctx = create_ssl_context();
        if (!ssl_ctx) {
            close(sockfd);
            return 1;
        }

        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, sockfd);

        if (SSL_connect(ssl) != 1) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
            close(sockfd);
            return 1;
        }
    }

    // Send HTTP GET request
    if (is_https) {
        SSL_write(ssl, "GET /", 5);
        SSL_write(ssl, path, strlen(path));
        SSL_write(ssl, " HTTP/1.1\r\nHost: ", 17);
        SSL_write(ssl, hostname, strlen(hostname));
        SSL_write(ssl, "\r\n\r\n", 4);
    } else {
        dprintf(sockfd, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);
    }

    // Open the output file with the extracted filename
    const char *filename = extract_filename(url);
    const char *downPath = loadDownFolderPath();

    printf("%s\n", filename);
    printf("%s\n", downPath);

    char finalPath[1024];
    strcpy(finalPath,downPath);
    strcat(finalPath,filename);
    printf("%s\n", finalPath);
    logger(finalPath, true);
    FILE *file = fopen(finalPath, "wb");
    if (file == NULL) {
        perror("Error opening output file");
        if (is_https) {
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
        }
        close(sockfd);
        return 1;
    }

    // Receive and save the file data
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    if (is_https) {
        bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
    } else {
        bytesRead = read(sockfd, buffer, sizeof(buffer));
    }

    while (bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, file);
        if (is_https) {
            bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        } else {
            bytesRead = read(sockfd, buffer, sizeof(buffer));
        }
    }

    // Clean up
    fclose(file);
    if (is_https) {
        SSL_free(ssl);
        SSL_CTX_free(ssl_ctx);
    }
    close(sockfd);

    printf("Download complete. File saved as %s\n", filename);

    return 0;
}

int main() {
    bool prvy = true;
    int choice = 0;
    int state;
    while (choice != 5) {
        //ui

        //exit
        if (choice != 5) {

            //download manazer
            if (prvy) {
                printf("\nWelcome to POS Download Manager! \n Choose the action:\n");
            } else {
                printf("Choose the next action:\n");
            }
            printf("1 - download file\n");
            printf("2 - schedule download for later\n");
            printf("3 - manage download directory\n");
            printf("4 - view download history\n");
            printf("5 - exit :(\n");

            scanf("%d", &choice);

            // Buffer to store user input
            char url_buffer[256];

            if (choice > 0 && choice < 3) {
                //timer
                int timer = 0;
                //printf((const char *) choice);
                if (choice == 2) {
                    printf("\nzadaj za aky cas v minutach ma stahovanie zacat:\n");
                    scanf("%d", &timer);
                    fflush(stdin);
                }

                //čistenie terminálu
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                // Prompt the user for the URL
                printf("Enter the URL: ");
                if (fgets(url_buffer, sizeof(url_buffer), stdin) == NULL) {
                    fprintf(stderr, "Error reading input\n");
                    return 1;
                }

                // Remove newline character from the end of the URL
                size_t len = strlen(url_buffer);
                if (len > 0 && url_buffer[len - 1] == '\n') {
                    url_buffer[len - 1] = '\0';
                }

                // URL of the file to download
                const char *url = url_buffer;

                // Download the file
                if (download_file(url, timer) != 0) {
                    fprintf(stderr, "Error downloading file\n");
                    return 1;
                }
            }

            if (choice == 3) {
                FILE *file;
                const char *filename = "downFolderPath.txt";
                char userInput[1000]; // Assuming a maximum of 999 characters for a single line

                // Open the file in write mode ("w")
                file = fopen(filename, "w");

                // Check if the file was opened successfully
                if (file == NULL) {
                    perror("Error opening file");
                    return 1; // Exit with an error code
                }

                // Get user input
                printf("Enter a directory where manager should be downloading:\n");
                fgets(userInput, sizeof(userInput), stdin);

                // Clear the input buffer (discard remaining characters in the buffer)
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                // Write user input to the file
                fprintf(file, "%s", userInput);

                // Close the file
                fclose(file);

            }

            if (choice == 4){
                logger( NULL, false);
            }
            //TOTO CISTI INPUT BUFFER!!! BEZ TOHTO SA ZADRHAVA NACITANIE! POUZIVAT ZA KAZDYM NACITANIM Z KLAVESNICE!!!
//    int c;
//    while ((c = getchar()) != '\n' && c != EOF);

            //return 0;

        }
        prvy = false;
    }
    return state;
}
