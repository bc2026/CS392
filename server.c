// I pledge my honor that I have abided by the Stevens Honor System.
// Bhagawat Chapagain

#define _POSIX_C_SOURCE 2
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "question.txt"
#define DEFAULT_PORT "25555"
#define MAX_ENTRIES 50
#define MAX_CLIENTS 3

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

int read_questions(struct Entry *arr, char *filename);
uint16_t conv_char_to_uint16(const char *);
void *handle_client(void *arg, int client_sockets[MAX_CLIENTS], int active_connections);

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

// This is a pseudo-function you would call to handle a client
void *handle_client(void *arg, int client_sockets[MAX_CLIENTS], int active_connections)
{
    int index = *(int *)arg; // Cast and dereference the pointer to get the index
    char buffer[256];

    puts("Please type your name: \n");
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_sockets[index], buffer, sizeof(buffer) - 1);

        if (n < 0)
        {
            perror("ERROR reading from socket");
            break;
        }
        else if (n == 0)
        {
            // Client has closed the connection
            printf("Lost connection");
            break;
        }

        // Echo the received message back to the client

        n = write(client_sockets[index], buffer, strlen(buffer));

        if (n < 0)
        {
            perror("ERROR writing to socket");
            break;
        }

        printf("Hi %s!", buffer);
    }
    close(client_sockets[index]);
    client_sockets[index] = -1;
    active_connections--;

    return NULL; // Return NULL upon thread completion
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

int load_questions(const char *filename, Entry entries[])
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
    const char *ip = DEFAULT_IP;
    const char *file = DEFAULT_FILE;
    const char *port = DEFAULT_PORT;

    char error[1024] = {0};
    char usage[1024] = "Usage: ";
    char *first_arg = strdup(argv[0]);

    char usage_message[] = " \
    [-f question_file] [-i IP_address] [-p port_number] [-h]\n\n\
    \t- f question_file\tDefault to \"question.txt\";\n\
    \t- i IP_address\t\tDefault to \"127.0.0.1\";\n\
    \t- p port_number\t\tDefault to 25555;\n\
    \t- h            \t\tDisplay this help info.";

    if (first_arg)
    {
        strcat(usage, first_arg);
        free(first_arg);
    }

    strcat(usage, usage_message);

    char opt;

    while ((opt = getopt(argc, argv, ":f:i:ph")) != -1)
    {
        switch (opt)
        {
        case 'h':
            puts(usage);
            return 0;
        case 'f':
            file = optarg;
            break;
        case 'i':
            ip = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        case '?':
            snprintf(error, sizeof(error), "Error: Unknown option '-%c' received.\n", optopt);
            fputs(error, stderr);
            return EXIT_FAILURE;
        }
    }

    // Handle non-option arguments here
    // for (int index = optind; index < argc; index++)
    // {
    //     printf("Non-option argument: %s\n", argv[index]);
    // }

    // printf("%s, %s, %s\n", file, ip, port);

    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    // bind socket
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // Clear structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any IP address
    serv_addr.sin_port = htons(conv_char_to_uint16(port));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    // listen for client

    if (listen(sockfd, MAX_CLIENTS) == -1)
    {
        perror("Listen failed. Exiting... ");
        return 1;
    }

    puts("Welcome to 392 Trivia!");

    Entry entries[MAX_ENTRIES];
    int num_entries = load_questions("questions.txt", entries);

    // // Example usage: Print loaded questions and answers
    // for (int i = 0; i < num_entries; i++)
    // {
    //     printf("Question: %s\n", entries[i].prompt);
    //     printf("Options: %s, %s, %s\n", entries[i].options[0], entries[i].options[1], entries[i].options[2]);
    //     printf("Correct answer: %s\n", entries[i].options[entries[i].answer_idx]);
    //     printf("\n");
    // }

    // accept connections
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    pthread_t threads[MAX_CLIENTS];
    int indices[MAX_CLIENTS]; // Store indices for thread arguments

    int client_sockets[MAX_CLIENTS];
    int active_connections = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = -1; // Initialize all entries to -1 indicating unused slot
    }

    while (1)
    {
        if (active_connections < MAX_CLIENTS)
        {
            int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0 && errno != EINTR)
            {
                perror("ERROR on accept");
                continue;
            }

            // Print client connection details
            puts("New connection detected!");

            int found = 0;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == -1)
                { // Find a free slot
                    client_sockets[i] = newsockfd;
                    active_connections++;
                    indices[i] = i; // Prepare the index for the thread

                    // Create a new thread for each client
                    if (pthread_create(&threads[i], NULL, handle_client, &indices[i]) != 0)
                    {
                        perror("Failed to create thread");
                    }
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                printf("Failed to find a free slot for new client\n");
                close(newsockfd); // Close the socket if no free slot is found
            }
        }
        else
        {
            // Maximum clients reached, you can decide to close the socket
            // or handle it according to your application needs
            printf("Maximum client connections reached. New connections will be temporarily blocked.\n");
            sleep(1); // Sleep to prevent busy waiting
        }
    }

    // Join threads and clean up (not shown)
    close(sockfd);
    return 0;

    puts("The game starts!");
}