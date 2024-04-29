#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void display_help(const char *program_name)
{
    printf("Usage: %s [-i IP_address] [-p port_number] [-h]\n", program_name);
    printf("\t-i IP_address   Default to \"127.0.0.1\";\n");
    printf("\t-p port_number  Default to 25555;\n");
    printf("\t-h              Display this help info.\n");
}

void parse_connect(int argc, char **argv, int *server_fd)
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
            if (port == 0)
            {
                fprintf(stderr, "Error: Invalid port number '%s'\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            display_help(argv[0]);
            exit(EXIT_SUCCESS);
        default: // '?'
            fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
            exit(EXIT_FAILURE);
        }
    }

    // Create a socket
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int server_fd;
    parse_connect(argc, argv, &server_fd);

    // Following could be the logic to handle communication or further processing
    // close(server_fd);

    return 0;
}
