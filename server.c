#define _POSIX_C_SOURCE 2
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "question.txt"
#define DEFAULT_PORT "25555"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    return 0;
}
