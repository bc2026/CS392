#define _GNU_SOURCE
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "questions.txt"
#define DEFAULT_PORT 25555
#define MAX_ENTRIES 50
#define NAME_SIZE 16
#define MAX_CLIENTS 3
#define BUFLEN 1024

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
#include <pthread.h>
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
#include <netinet/in.h>

typedef struct Entry
{
    char prompt[1024];
    char options[3][50];
    int answer_idx;
} Entry;

void start_game()
{
    return;
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

void accept_players(int server_fd)
{
    int client_socket[MAX_CLIENTS], max_clients = MAX_CLIENTS, i, new_socket, addrlen = sizeof(struct sockaddr_in);
    char buffer[1024]; // Buffer to store input
    fd_set readfds;
    struct sockaddr_in address;

    // Initialize all client_socket[] to 0 so they are not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    printf("Waiting for connections ...\n");

    while (1)
    {
        FD_ZERO(&readfds);           // Clear the socket set
        FD_SET(server_fd, &readfds); // Add master socket to set
        int max_sd = server_fd;

        // Add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            int sd = client_socket[i];
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        // Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("Select error: %s\n", strerror(errno));
            continue;
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                continue;
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    // Send a welcome message to the client
                    const char *message = "Please type your name: ";
                    if (send(new_socket, message, strlen(message), 0) < strlen(message))
                    {
                        perror("Failed to send full message");
                    }
                    break;
                }
            }
        }

        // Handle I/O for each socket
        for (i = 0; i < max_clients; i++)
        {
            int sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing, and also read the incoming message
                int valread = recv(sd, buffer, sizeof(buffer), 0);
                if (valread == 0)
                {
                    // Somebody disconnected, get his details and print
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected, ip %s, port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                else if (valread > 0)
                {
                    buffer[valread] = "\0";
                    char hi_msg[BUFLEN];

                    sprintf(hi_msg, "Hi %s!", buffer);

                    if (send(new_socket, hi_msg, strlen(hi_msg), 0) < strlen(hi_msg))
                    {
                        perror("Failed to send full message");
                    }
                    // Here you can handle the received name and perhaps send more commands or information
                }
                else if (valread < 0)
                {
                    perror("recv error");
                }
            }
        }
    }
}
