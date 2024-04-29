#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void display_help(const char *program_name)
{
    printf("Usage: %s [-i IP_address] [-p port_number] [-h]\n", program_name);
    printf("\t-i IP_address   Default to \"127.0.0.1\";\n");
    printf("\t-p port_number  Default to 25555;\n");
    printf("\t-h              Display this help info.\n");
}

int connect_to_server(const char *ip_address, int port)
{
    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IPv4 or IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

void interact_with_server(int server_fd)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        // Clear the buffer and read message from the server
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(server_fd, buffer, BUFFER_SIZE - 1);
        if (bytes < 0)
        {
            perror("read");
            break;
        }
        else if (bytes == 0)
        {
            printf("Server closed the connection.\n");
            break;
        }

        printf("Server: %s\n", buffer);

        // Handle server messages here. For example, if the server asks for the player's name,
        // you should send your name to the server.

        // If this is a question, prompt the user for input and send the answer to the server.
        if (strstr(buffer, "Question") != NULL)
        {
            printf("Your answer: ");
            fflush(stdout); // Ensure "Your answer" is printed before reading input

            memset(buffer, 0, sizeof(buffer));
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
            {
                perror("fgets");
                break;
            }

            // Send the answer to the server
            if (write(server_fd, buffer, strlen(buffer)) < 0)
            {
                perror("write");
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{
    const char *ip_address = "127.0.0.1";
    int port = 25555;
    int opt;

    while ((opt = getopt(argc, argv, "i:p:h")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip_address = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'h':
            display_help(argv[0]);
            exit(EXIT_SUCCESS);
        default:
            display_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Connect to the server
    int server_fd = connect_to_server(ip_address, port);

    // Interaction loop
    interact_with_server(server_fd);

    // Clean up
    close(server_fd);
    return 0;
}