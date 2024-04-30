#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>

#define MIN_PORT 1024
#define MAX_PORT 65535
#define MAX_NAME_LEN 20
#define BUFLEN 1024

volatile sig_atomic_t running = true;

void catch_signal(int sig)
{
    running = false;
    putchar('\n');
}

int receive_message(int client_socket, char *inbuf)
{
    memset(inbuf, 0, BUFLEN);
    int bytes_received = recv(client_socket, inbuf, BUFLEN - 1, 0);
    if (bytes_received < 0)
    {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
        return -1;
    }
    else if (bytes_received == 0)
    {
        printf("Server closed the connection.\n");
        return 0;
    }
    inbuf[bytes_received] = '\0';
    return bytes_received;
}

int send_message(int client_socket, const char *outbuf)
{
    int bytes_sent = send(client_socket, outbuf, strlen(outbuf), 0);
    if (bytes_sent < 0)
    {
        fprintf(stderr, "Error: Failed to send message to server. %s.\n", strerror(errno));
    }
    return bytes_sent;
}

void print_prompt(const char *username)
{
    printf("[%s]: ", username);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int client_socket, port;
    struct sockaddr_in server_addr;
    const char *server_ip = argv[1];
    port = atoi(argv[2]);

    if (port < MIN_PORT || port > MAX_PORT)
    {
        fprintf(stderr, "Error: Port must be in range [%d, %d].\n", MIN_PORT, MAX_PORT);
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", server_ip);
        return EXIT_FAILURE;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
        close(client_socket);
        return EXIT_FAILURE;
    }

    char inbuf[BUFLEN];
    while (running)
    {
        if (receive_message(client_socket, inbuf) > 0)
        {
            printf("%s\n", inbuf);

            if (strstr(inbuf, "Please type your name:"))
            {
                char name[BUFLEN];
                fgets(name, BUFLEN, stdin);        // Get the player's name from stdin
                send_message(client_socket, name); // Send the name back to the server
            }

            // Improved recognition of questions and options
            if (strstr(inbuf, "Options:"))
            {
                printf("Your answer: ");
                char answer[BUFLEN];
                fgets(answer, BUFLEN, stdin);
                send_message(client_socket, answer);
            }
        }
        else
        {
            break;
        }
    }

    close(client_socket);
    return EXIT_SUCCESS;
}
