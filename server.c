// I pledge my honor that I have abided by the Stevens Honor System.
// Bhagawat Chapagain

#define _POSIX_C_SOURCE 2
#define _GNU_SOURCE
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "questions.txt"
#define DEFAULT_PORT "25555"
#define MAX_ENTRIES 50
#define MAX_CLIENTS 3
#define NAME_SIZE 16

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

struct ClientData
{
    int client_socket;
    int *active_connections;
    struct Player *players;
    int index;
};

struct Player
{
    int fd;
    int score;
    char name[128];
};

typedef struct Entry
{
    char prompt[1024];
    char options[3][50];
    int answer_idx;
} Entry;

int read_questions(Entry *entries, const char *filename);
uint16_t conv_char_to_uint16(const char *);
void *handle_client(void *arg);

void *handle_client(void *arg)
{
    struct ClientData *data = (struct ClientData *)arg;
    int client_socket = data->client_socket;
    int *active_connections = data->active_connections;

    if (data->index < 0 || data->index >= MAX_CLIENTS)
    {
        fprintf(stderr, "Index out of bounds\n");
        return NULL;
    }

    // Check for a valid socket descriptor
    if (data->client_socket < 0)
    {
        fprintf(stderr, "Invalid socket descriptor\n");
        return NULL;
    }

    // Greet the client
    const char *greeting = "Please type your name: ";
    send(data->client_socket, greeting, strlen(greeting), 0);

    // Receive the name from the client
    char buffer[256] = {0};
    ssize_t bytes_read = recv(data->client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0)
    {
        // Handle disconnection properly without exiting the whole server
        printf("User disconnected or error occurred\n");
        *(active_connections)--;
        close(client_socket);
        return NULL;
    }

    // Null-terminate the received data and store the name
    buffer[bytes_read] = '\0';
    strncpy(data->players[data->index].name, buffer, NAME_SIZE - 1);

    // Send confirmation back to the client
    char confirm[NAME_SIZE + 32];
    snprintf(confirm, sizeof(confirm), "Hi %s! You're now connected.\n", data->players[data->index].name);
    send(data->client_socket, confirm, strlen(confirm), 0);

    // TODO: Handle further client-server communication

    // Close the client socket and end the thread
    close(data->client_socket);
    return NULL;
}

uint16_t conv_char_to_uint16(const char *str)
{
    char *end;
    uint16_t value = (uint16_t)strtoul(str, &end, 10); // Base 10 for decimal

    if (*end != '\0')
    {
        // printf("Conversion error, non-integer data in input string\n");
        return EXIT_FAILURE;
    }

    return value;
}

int read_questions(Entry *entries, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file");
        return -1; // Return -1 to indicate file opening error
    }

    int entry_count = 0;
    char line[1024];
    while (entry_count < MAX_ENTRIES)
    {
        // Read the question
        if (!fgets(line, sizeof(line), file) || strlen(line) == 1)
        {
            break; // Break if reading the question fails or line is empty
        }
        line[strcspn(line, "\n")] = 0;
        strncpy(entries[entry_count].prompt, line, sizeof(entries[entry_count].prompt));

        // Read options line
        if (!fgets(line, sizeof(line), file))
        {
            break; // Break if reading options fails
        }
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "%49s %49s %49s", entries[entry_count].options[0], entries[entry_count].options[1], entries[entry_count].options[2]);

        // Read correct answer line
        if (!fgets(line, sizeof(line), file))
        {
            break; // Break if reading the correct answer fails
        }
        line[strcspn(line, "\n")] = 0;

        // Determine the correct answer index
        int found = 0;
        for (int i = 0; i < 3; i++)
        {
            if (strcmp(entries[entry_count].options[i], line) == 0)
            {
                entries[entry_count].answer_idx = i;
                found = 1;
                break;
            }
        }
        if (!found)
        {
            fprintf(stderr, "Error: Correct answer '%s' not found among options for question %d\n", line, entry_count + 1);
        }

        // Optionally skip a blank line after each entry if present
        fgets(line, sizeof(line), file);

        entry_count++;
    }

    fclose(file);
    return entry_count; // Return the number of entries read
}
int main(int argc, char **argv)
{
    // Set default values
    const char *ip = DEFAULT_IP;
    const char *port = DEFAULT_PORT;
    const char *file = DEFAULT_FILE;

    // Process command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "f:i:p:h")) != -1)
    {
        switch (opt)
        {
        case 'f': // Question file
            file = optarg;
            break;
        case 'i': // IP address
            ip = optarg;
            break;
        case 'p': // Port number
            port = optarg;
            break;
        case 'h': // Help
            // Display usage
            printf("Usage: %s [-f question_file] [-i IP_address] [-p port_number] [-h]\n", argv[0]);
            exit(EXIT_SUCCESS);
        case '?': // Unknown option
            // Display error message and exit
            fprintf(stderr, "Unknown option: %c\n", optopt);
            exit(EXIT_FAILURE);
        }
    }

    // Create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Set server address and bind
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(conv_char_to_uint16(port));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }

    // Start listening on the socket
    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }

    // Load the questions from the file
    Entry entries[MAX_ENTRIES];
    int num_entries = read_questions(entries, file);
    if (num_entries < 0)
    {
        // Handle error if questions couldn't be loaded
        fprintf(stderr, "Failed to load questions from the file.\n");
        exit(EXIT_FAILURE);
    }

    // Main loop to accept clients
    struct Player players[MAX_CLIENTS];
    struct ClientData client_data[MAX_CLIENTS];
    pthread_t threads[MAX_CLIENTS];
    int active_connections = 0;

    while (1)
    {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            continue; // Continue to the next iteration of the loop
        }

        // New client connection logic
        printf("New connection detected!\n");

        // Allocate ClientData on heap for thread persistence
        struct ClientData *cd = malloc(sizeof(struct ClientData));
        if (!cd)
        {
            perror("malloc");
            close(newsockfd);
            continue;
        }

        cd->client_socket = newsockfd;
        cd->active_connections = &active_connections;
        cd->players = players;          // Ensure players is allocated and valid
        cd->index = active_connections; // Assuming active_connections is next free index

        if (pthread_create(&threads[active_connections], NULL, handle_client, cd) != 0)
        {
            perror("Failed to create thread");
            free(cd);
            close(newsockfd);
        }
        else
        {
            active_connections++; // Only increment if thread creation is successful
        }

        if (active_connections >= MAX_CLIENTS)
        {
            // Maximum clients reached
            printf("Max connections reached.\n");
        }
    }

    // Cleanup code for server shutdown
    for (int i = 0; i < active_connections; i++)
    {
        pthread_join(threads[i], NULL); // Wait for threads to finish
    }
    close(sockfd); // Close server socket

    puts("The game starts!");

    // // Example usage: Print loaded questions and answers
    // for (int i = 0; i < num_entries; i++)
    // {
    //     printf("Question: %s\n", entries[i].prompt);
    //     printf("Options: %s, %s, %s\n", entries[i].options[0], entries[i].options[1], entries[i].options[2]);
    //     printf("Correct answer: %s\n", entries[i].options[entries[i].answer_idx]);
    //     printf("\n");
    // }

    int winner = -1;

    for (size_t i = 0; i < MAX_ENTRIES; i++)
    {
        printf("Question <%d>: %s\n", i, entries[i].prompt);
        printf("PRESS 1: %s\n", entries[i].options[0]);
        printf("PRESS 2: %s\n", entries[i].options[1]);
        printf("PRESS 3: %s\n", entries[i].options[2]);

        // TODO: catch input and change each players[i] accordingly:
        // a player answered the question right, they get one point; otherwise they get -1 pts

        printf("Answer: %s\n", entries[i].options[entries[i].answer_idx]);

        if (i == MAX_ENTRIES - 1)
        {
            int max = 0;
            for (size_t i = 0; i < MAX_CLIENTS; i++)
            {
                if (players[i].score > max)
                {
                    max = players[i].score;
                    winner = i;
                }
            }

            printf("Congrats %s!", players[winner].name);
            break;
        }
    }

    return 0;
}