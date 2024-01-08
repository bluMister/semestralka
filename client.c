#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define MAX_VLAKIEN 100

// Structure for thread information
typedef struct {
    pthread_t id;
    int timer;
    const char *url;
} VlaknoInfo;

//download logger
void logger(char *log, bool write) {

    if (write) {
        time_t t;
        struct tm *current_time;
        char buffer[1024];

        time(&t);
        current_time = localtime(&t);

        // Format the date and time
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", current_time);

        strcat(buffer, log);

        // Open a file for writing
        FILE *file = fopen("logger.txt", "a");

        if (file == NULL) {
            printf("Error opening log file!\n");
        }

        // Write the formatted date and time to the file
        fprintf(file, "%s\n", buffer);

        // Close the file
        fclose(file);
    } else {
        FILE *file = fopen("logger.txt", "r");

        if (file == NULL) {
            printf("Error opening log file for reading!\n");
        }

        // Read and display the contents of the file
        printf("current download history:\n");

        char line[1024];
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }
        // Close the file
        fclose(file);
    }

}

// Function to extract hostname and path from the URL
int parse_url(const char *url, char *hostname, char *path, bool *is_https) {
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
bool directoryExists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}
//function to read download folder path from file
char *downFolderPath(char *newPath, bool write) {

    if (write) {
        char buffer[1024];

        // Open a file for writing
        FILE *file = fopen("downFolderPath.txt", "w");

        if (file == NULL) {
            printf("Error opening path file!\n");
        }

        strcpy(buffer, newPath);

        // Write to the file
        fprintf(file, "%s\n", buffer);

        // Close the file
        fclose(file);

        return newPath;

    } else {
        FILE *file = fopen("downFolderPath.txt", "r");

        if (file == NULL) {
            printf("Error opening path file for reading!\n");
        }

        // Read and display the contents of the file
        printf("Current download folder path: ");

        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline(&line, &len, file)) != -1) {
            printf("%s\n", line);
        }

        // Close the file
        fclose(file);

        // Trim the newline character from the end of the line
        if (line != NULL && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        return line;
    }

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

// Function to extract ftpHost and ftpPath from the full FTP URL
void parseFtpUrl(const char *fullUrl, char *ftpHost, char *ftpPath) {
    // Check if the URL starts with "ftp://"
    if (strncmp(fullUrl, "ftp://", 6) != 0) {
        fprintf(stderr, "Invalid FTP URL. It should start with 'ftp://'\n");
        ftpHost[0] = '\0';
        ftpPath[0] = '\0';
        return;
    }

    // Skip the "ftp://" prefix
    const char *urlWithoutPrefix = fullUrl + 6;

    // Find the first slash to separate ftpHost and ftpPath
    const char *slashPosition = strchr(urlWithoutPrefix, '/');
    if (slashPosition == NULL) {
        fprintf(stderr, "Invalid FTP URL. Missing path after 'ftp://'\n");
        ftpHost[0] = '\0';
        ftpPath[0] = '\0';
        return;
    }

    // Calculate the lengths of ftpHost and ftpPath
    size_t hostLength = slashPosition - urlWithoutPrefix;
    size_t pathLength = strlen(urlWithoutPrefix) - hostLength;

    // Copy ftpHost and ftpPath to the provided buffers
    strncpy(ftpHost, urlWithoutPrefix, hostLength);
    ftpHost[hostLength] = '\0';

    strncpy(ftpPath, slashPosition, pathLength);
    ftpPath[pathLength] = '\0';
}

// Main thread function, processes the file download
void *vlaknoFunkcia(void *arg) {
    VlaknoInfo *info = (VlaknoInfo *) arg;

    char hostname[1024];
    char path[1024];
    bool is_https;

    if (strncmp(info->url, "ftp://", 6) == 0) {
        printf("downloading an FTP file'\n");

        char ftpHost[BUFFER_SIZE];
        char ftpPath[BUFFER_SIZE];

        // Parse the FTP URL
        parseFtpUrl(info->url, ftpHost, ftpPath);

        // Check if parsing was successful
        if (ftpHost[0] == '\0' || ftpPath[0] == '\0') {
            perror("Error parsing the host");
        }

        struct hostent *host = gethostbyname(ftpHost);
        if (host == NULL) {
            perror("Error resolving host");
        }

        // Create socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("Error creating socket");
        }

        // Initialize server address structure
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(21);
        memcpy(&serverAddr.sin_addr.s_addr, host->h_addr, host->h_length);

        // Connect to the server
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Error connecting to server");
            close(sockfd);
        }

        // Read welcome message
        char response[BUFFER_SIZE];
        if (read(sockfd, response, sizeof(response)) == -1) {
            perror("Error reading from server");
            close(sockfd);
        }
        printf("%s", response);

        // Log in
        if (strstr(response, "220 FTP Server") != NULL) {
            char username[BUFFER_SIZE];
            char password[BUFFER_SIZE];

            // Prompt user for username and password
            printf("Enter FTP username: ");
            scanf("%s", username);
            printf("Enter FTP password: ");
            scanf("%s", password);

            // Log in
            char loginCommand[BUFFER_SIZE];
            snprintf(loginCommand, sizeof(loginCommand), "USER %s\r\n", username);
            write(sockfd, loginCommand, strlen(loginCommand));

            snprintf(loginCommand, sizeof(loginCommand), "PASS %s\r\n", password);
            write(sockfd, loginCommand, strlen(loginCommand));
        }

        // Change to binary mode
        write(sockfd, "TYPE I\r\n", strlen("TYPE I\r\n"));

        // Request the file
        char fileCommand[BUFFER_SIZE];
        snprintf(fileCommand, sizeof(fileCommand), "RETR %s\r\n", ftpPath);
        write(sockfd, fileCommand, strlen(fileCommand));

        // Open the output file with the extracted filename
        const char *filename = extract_filename(info->url);
        const char *downPath = downFolderPath(NULL, false);

        printf("Downloading of %s has started!\n", filename);

        char finalPath[1024];

        strcpy(finalPath, downPath);
        strcat(finalPath, filename);
        printf("%s\n", finalPath);

        logger( finalPath, true);

        FILE *file = fopen(finalPath, "wb");
        if (file == NULL) {
            perror("Error opening local file for writing");
            close(sockfd);
        }

        // Read and write file data
        ssize_t bytesRead;

        printf("download is starting\n");

        while ((bytesRead = read(sockfd, response, sizeof(response))) > 0) {
            fwrite(response, 1, bytesRead, file);
        }

        if (bytesRead == -1) {
            perror("Error reading from server");
        }

        // Check if bytesRead is 0, indicating the end of the file
        if (bytesRead == 0) {
            printf("Download complete. File saved as %s\n", finalPath);
        } else {
            printf("Download incomplete or failed.\n");
        }

        // Close local file and socket
        fflush(file);
        fclose(file);
        close(sockfd);

        //printf("Download complete. File saved as %s\n", finalPath);

        pthread_exit(NULL);
    }

    // Parse URL to extract hostname and path
    if (parse_url(info->url, hostname, path, &is_https) != 0) {
    }

    //check for timer and wait if necessary
    if (info->timer > 0) {
        sleep(info->timer * 60);
    }

    // Resolve hostname to IP address
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        perror("Error resolving hostname");
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
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
    }

    // If using HTTPS, set up SSL
    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;
    if (is_https) {
        SSL_library_init();
        ssl_ctx = create_ssl_context();
        if (!ssl_ctx) {
            close(sockfd);
        }

        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, sockfd);

        if (SSL_connect(ssl) != 1) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
            close(sockfd);
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
    const char *filename = extract_filename(info->url);
    const char *downPath = downFolderPath(NULL, false);

      printf("Downloading of %s has started!\n", filename);

    char finalPath[1024];

    strcpy(finalPath, downPath);
    strcat(finalPath, filename);
    printf("%s\n", finalPath);

    logger( finalPath, true);

    FILE *file = fopen(finalPath, "wb");

    if (file == NULL) {
        perror("Error opening local copy of file");
        if (is_https) {
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
        }
        close(sockfd);
    }

    // Receive and save the file data
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    printf("omba\n");

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

    printf("Download complete. File saved as %s\n", finalPath);

    //End of thread
    pthread_exit(NULL);

}

int main() {
    VlaknoInfo vlakna[MAX_VLAKIEN];
    int pocetVlakien = 0;


    bool prvy = true; //Welcome text only at the beginning of the program
    int choice = 0;
    while (choice != 5) {
        if (choice != 5) {

            //download manager
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
            //console clean up, catching stuck input from keyboard
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }

            // Buffer to store user input
            char url_buffer[1024];

            if (choice > 0 && choice < 3 && pocetVlakien <= MAX_VLAKIEN) {

                int timer = 0;

                if (choice == 2) {
                    printf("Set time in minutes after which the downloading will begin: ");
                    scanf("%d", &timer);
                    //console clean up, catching stuck input from keyboard
                    int d;
                    while ((d = getchar()) != '\n' && d != EOF) { }
                }

                // Prompt the user for the URL
                printf("Enter URL of the file you want to download: ");
                if (fgets(url_buffer, sizeof(url_buffer), stdin) == NULL) {
                    fprintf(stderr, "Error reading the URL input\n");
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
                int index = 0;
                while (vlakna[index].id != 0 && index < MAX_VLAKIEN) {
                    index++;
                }

                if (index < MAX_VLAKIEN) {
                    vlakna[index].timer = timer;
                    vlakna[index].url = strdup(url);

                    if (pthread_create(&vlakna[index].id, NULL, vlaknoFunkcia, &vlakna[index]) != 0) {
                        fprintf(stderr, "Error with creating a download thread\n");
                        free((void *)vlakna[index].url);
                        return 1;
                    }

                    pocetVlakien++;
                } else {
                    printf("Too many files are being downloaded to start new download thread!\n");
                }

            }
            if (pocetVlakien > MAX_VLAKIEN) {
                printf("Maximum amount of download threads reached! \nWait with next download for previous ones to finish!\n");
            }

            //------------------------------------------------3333333----------------------------------------------
            if (choice == 3) {

                char input[1024]; // Assuming a fixed-size buffer for input

                downFolderPath(NULL, false);
                printf("Enter path to download folder: ");

                if (fgets(input, sizeof(input), stdin) != NULL) {
                    // Remove the newline character at the end, if it exists
                    char *newline = strchr(input, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }

                } else {
                    fprintf(stderr, "Error reading input for download path\n");
                    return 1;
                }

                if (!directoryExists(input)) {
                    printf("Directory of download path missing! Creating missing directory...\n");
                    // Create the directory if it doesn't exist
                    if (mkdir(input, 0777) == -1) {
                        // Check if there was an error creating the directory
                        printf("Error creating corresponding directory.\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("Directory created successfully.\n");
                }

                downFolderPath(input, true);

            }

            //------------------------------------------------44444444------------------------------------------------
            if (choice == 4) {
                logger(NULL, false);
            }
        }
        prvy = false;


    }

    printf("Application is terminating! Waiting for remaining downloads to finish...\n");
    // Waiting for threads to finish
    for (int i = 0; i < pocetVlakien; ++i) {
        if (pthread_join(vlakna[i].id, NULL) != 0) {
            fprintf(stderr, "Error while waiting for download thread\n");
            return 1;
        }
        free((void *)vlakna[i].url);
    }

    return 0;
}
