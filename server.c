// I pledge my honor that I have abided by the Stevens Honor System.
// Bhagawat Chapagain
#define _POSIX_C_SOURCE 2
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "question.txt"
#define DEFAULT_PORT "25555"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

uint16_t conv_char_to_uint16(const char *);

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

    printf("%s, %s, %s\n", file, ip, port);

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
    listen(sockfd, 5);

    // intermediate steps
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        exit(1);
    }

    char buffer[256];
    memset(buffer, 0, 256); // Clear buffer

    int n = read(newsockfd, buffer, 255);
    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n", buffer);

    n = write(newsockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // close socket
    close(newsockfd);
    close(sockfd);
}
