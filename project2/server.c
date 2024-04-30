#define _POSIX_C_SOURCE 2
#define _GNU_SOURCE
#define MAX_ENTRIES 50
#define BUFLEN 2048
#define MAX_CLIENTS 3
#define DEFAULT_FILE "questions.txt"
#define DEFAULT_PORT 25555
#define DEFAULT_IP "127.0.0.1"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

typedef struct Entry
{
    char prompt[1024];
    char options[3][50];
    int answer_idx;
} Entry;

typedef struct Player
{
    int fd;
    int score;
    char name[128];
} Player;

void start_game(Player *players, Entry *questions)
{
    char buffer[BUFLEN]; // Buffer to hold the full message to send

    // Loop through each question
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        // Check if the question has been loaded correctly (non-empty prompt)
        if (strlen(questions[i].prompt) == 0)
        {
            continue; // Skip if the prompt is empty
        }

        // Prepare the full message to send for current question
        snprintf(buffer, sizeof(buffer), "%s\nOptions: %s, %s, %s\n",
                 questions[i].prompt,
                 questions[i].options[0],
                 questions[i].options[1],
                 questions[i].options[2]);

        // Send the question and options to each connected client
        for (int j = 0; j < MAX_CLIENTS; j++)
        {
            if (players[j].fd != 0)
            { // Check if the socket is active
                if (send(players[j].fd, buffer, strlen(buffer), 0) < strlen(buffer))
                {
                    perror("Failed to send full message");
                }
            }
        }
    }
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

void accept_players(int server_fd, Player players[])
{
    struct sockaddr_in address;
    int addrlen = sizeof(struct sockaddr_in);
    char buffer[BUFLEN]; // Buffer to store input
    fd_set readfds;

    printf("Waiting for connections ...\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        players[i].fd = 0;    // Initialize all player file descriptors to 0
        players[i].score = 0; // Initialize scores to 0
    }

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (players[i].fd > 0)
            {
                FD_SET(players[i].fd, &readfds);
                if (players[i].fd > max_sd)
                {
                    max_sd = players[i].fd;
                }
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("Select error: %s\n", strerror(errno));
            continue;
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (new_socket < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (players[i].fd == 0)
                {
                    players[i].fd = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    const char *message = "Welcome to 392 Trivia!\nPlease type your name: ";
                    if (send(new_socket, message, strlen(message), 0) < strlen(message))
                    {
                        perror("Failed to send full message");
                    }

                    int valread = recv(new_socket, buffer, sizeof(buffer), 0);
                    if (valread > 0)
                    {
                        buffer[valread] = '\0'; // Ensure null termination
                        buffer[strcspn(buffer, "\n")] = '\0';
                        strcpy(players[i].name, buffer);

                        // Format the greeting message with the player's name
                        char hi_msg[BUFLEN];
                        sprintf(hi_msg, "Hi %s!", players[i].name); // Use sprintf to include the name in the greeting

                        // Send the personalized greeting back to the client
                        if (send(new_socket, hi_msg, strlen(hi_msg), 0) < strlen(hi_msg))
                        {
                            perror("Failed to send full message");
                        }
                    }
                    else
                    {
                        perror("recv failed or client disconnected");
                        close(new_socket);
                        players[i].fd = 0; // Reset fd in case of error
                    }
                    break;
                }
            }
        }
        // Check if all client slots are filled
        int all_filled = 1;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (players[i].fd == 0)
            {
                all_filled = 0;
                break;
            }
        }
        if (all_filled)
            break; // Break the loop if all clients are connected
    }
}

int main(int argc, char *argv[]) // Changed to non-const argv to use with getopt.
{
    const char *FILENAME = DEFAULT_FILE;
    unsigned short port = DEFAULT_PORT; // Changed to use a numerical port directly.
    const char *ip = DEFAULT_IP;

    // Process command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "f:i:p:h")) != -1)
    {
        switch (opt)
        {
        case 'f':
            FILENAME = optarg;
            break;
        case 'i':
            ip = optarg;
            break;
        case 'p':
            port = (unsigned short)atoi(optarg); // Convert port number from string to integer.
            break;
        case 'h':
            printf("Usage: %s [-f question_file] [-i IP_address] [-p port_number] [-h]\n", argv[0]);
            exit(EXIT_SUCCESS);
        case '?':
            fprintf(stderr, "Unknown option: -%c\n", optopt);
            exit(EXIT_FAILURE);
        }
    }

    // Create and bind a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip); // Convert IP address from string to binary.
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }

    const char *welcome_message = "Welcome to 392 Trivia!";
    puts(welcome_message);

    // Load questions
    Entry entries[MAX_ENTRIES];
    int numEntries = read_questions(entries, FILENAME);
    if (numEntries < 0)
    {
        fprintf(stderr, "Failed to load questions from the file.\n");
        exit(EXIT_FAILURE);
    }

    Player players[MAX_CLIENTS]; // Declare an array of Player structures

    accept_players(sockfd, players);
    start_game(players, entries); // Ensure `start_game` is adapted to use `Player` array
    return 0;

    return 0; // Properly return from main.
}
