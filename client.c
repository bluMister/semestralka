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

// Štruktúra pre uchovanie informácií o vlákne
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
        char buffer[100];

        time(&t);
        current_time = localtime(&t);

        // Format the date and time
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", current_time);

        strcat(buffer, log);

        // Open a file for writing
        FILE *file = fopen("downHistory.txt", "a");

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

    // Check if the file was opened successfully
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return NULL;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory to store the file content
    char *file_content = (char *)malloc(file_size + 1);

    // Check if memory allocation was successful
    if (file_content == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return NULL;
    }

    // Read the file content into the variable
    fread(file_content, 1, file_size, file);

    // Close the file
    fclose(file);

    // Null-terminate the content to treat it as a string
    file_content[file_size] = '\0';

    // Remove line breaks from the content
    for (int i = 0; i < file_size; i++) {
        if (file_content[i] == '\n' || file_content[i] == '\r') {
            file_content[i] = ' ';
        }
    }

    return file_content;
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

// Funkcia, ktorá bude vykonávaná vo vlákne
void *vlaknoFunkcia(void *arg) {
    VlaknoInfo *info = (VlaknoInfo *) arg;
    //printf("Vlákno s číslom %lu spustené. Spánok na %d sekúnd. URL: %s\n", info->id, info->timer, info->url);

    char hostname[256];
    char path[256];
    int is_https;

    // Parse URL to extract hostname and path
    if (parse_url(info->url, hostname, path, &is_https) != 0) {
        //return 1;
    }

    //check for timer and wiat if necessary
    if (info->timer > 0) {
        sleep(info->timer * 60);
    }

    // Resolve hostname to IP address
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        perror("Error resolving hostname");
        //return 1;
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        //return 1;
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
        //return 1;
    }

    // If using HTTPS, set up SSL
    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;
    if (is_https) {
        SSL_library_init();
        ssl_ctx = create_ssl_context();
        if (!ssl_ctx) {
            close(sockfd);
            //return 1;
        }

        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, sockfd);

        if (SSL_connect(ssl) != 1) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
            close(sockfd);
            //return 1;
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
    const char *downPath = loadDownFolderPath();

    printf("%s\n", filename);
    printf("%s\n", downPath);

    char finalPath[1024];
    strcpy(finalPath, downPath);
    strcat(finalPath, filename);
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
        //return 1;
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

    // ... vykonávajte ďalšie činnosti vo vlákne ...
    //printf("Vlákno s číslom %lu sa vykonalo. URL: %s\n", info->id, info->url);
    pthread_exit(NULL);

    return 0;
}

int main() {
    VlaknoInfo vlakna[MAX_VLAKIEN];
    int pocetVlakien = 0;


    bool prvy = true; //pre iny vypis na zaciaku a potom v procese behu programu
    int choice = 0;
    while (choice != 5) {
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

            if (choice > 0 && choice < 3 && pocetVlakien <= MAX_VLAKIEN) {

                int timer = 0;
                //printf((const char *) choice);
                if (choice == 2) {
                    printf("\nZadaj za aky cas v minutach ma stahovanie zacat:\n");
                    scanf("%d", &timer);
                    fflush(stdin);
                }

                //Čistenie terminálu
                while (getchar() != '\n');

                // Prompt the user for the URL
                printf("Zadajte URL: ");
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
//                    if (download_file(url, timer) != 0) {
//                        fprintf(stderr, "Error downloading file\n");
//                        return 1;
//                    }
                // Download the file
                int index = 0;
                while (vlakna[index].id != 0 && index < MAX_VLAKIEN) {
                    index++;
                }

                if (index < MAX_VLAKIEN) {
                    vlakna[index].timer = timer;
                    vlakna[index].url = strdup(url);

                    if (pthread_create(&vlakna[index].id, NULL, vlaknoFunkcia, &vlakna[index]) != 0) {
                        fprintf(stderr, "Chyba pri vytváraní vlákna.\n");
                        return 1;
                    }

                    pocetVlakien++;
                } else {
                    printf("Vektor vlákien je plný, nie je možné pridať ďalšie vlákno.\n");
                }
            }
            if(pocetVlakien > MAX_VLAKIEN){
                printf("Dosiahnuty maximalny pocet stahovanych suborov! \npockajte kým sa nejaký subor stiahne\n");
            }

            //------------------------------------------------3333333----------------------------------------------
            if (choice == 3) {

                char *current_content = loadDownFolderPath();
                if (current_content != NULL) {
                    printf("Current content of the file:\n%s\n", current_content);
                    free(current_content);  // Free the allocated memory
                }

                FILE *file = fopen("downFolderPath.txt", "a");

                // Check if the file was opened successfully
                if (file == NULL) {
                    fprintf(stderr, "Error opening file for writing\n");
                }

                // Prompt the user for input
                printf("Enter text (Enter to end input):\n");

                // Read input from the keyboard and write it to the file
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), stdin) != NULL && buffer[0] != '\n') {
                    fputs(buffer, file);
                }
                // Close the file
                fclose(file);

                // Clear the input buffer (discard remaining characters in the buffer)
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

            }

            //------------------------------------------------44444444------------------------------------------------
            if (choice == 4) {
                logger(NULL, false);
            }
        }
        prvy = false;


    }

    printf("program konci! cakanie na ukoncenie vlaken");
    // Čakanie na ukončenie vlákien
    for (int i = 0; i < pocetVlakien; ++i) {
        if (pthread_join(vlakna[i].id, NULL) != 0) {
            fprintf(stderr, "Chyba pri čakaní na vlákno.\n");
            return 1;
        }

        printf("Vlákno s číslom %d ukončené na konci. URL: %s\n", vlakna[i].timer, vlakna[i].url);

        // Uvoľnenie pamäte pre URL
        free((void *) vlakna[i].url);
    }

    return 0;
}
