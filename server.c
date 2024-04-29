// I pledge my honor that I have abided by the Stevens Honor System.
// Bhagawat Chapagain

#define _POSIX_C_SOURCE 2
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_FILE "question.txt"
#define DEFAULT_PORT "25555"
#define MAX_ENTRIES 50

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

uint16_t conv_char_to_uint16(const char *);
int read_questions(struct Entry *arr, char *filename);

typedef struct Entry
{
    char prompt[1024];
    char options[3][50];
    int answer_idx;
} Entry;

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

    int backlog = 3; // Limit the number of pending connections
    if (listen(sockfd, backlog) == -1)
    {
        perror("Listen failed. Exiting... ");
        return 1;
    }

    puts("Welcome to 392 Trivia!");

    Entry entries[MAX_ENTRIES];
    int num_entries = load_questions("questions.txt", entries);

    // Example usage: Print loaded questions and answers
    for (int i = 0; i < num_entries; i++)
    {
        printf("Question: %s\n", entries[i].prompt);
        printf("Options: %s, %s, %s\n", entries[i].options[0], entries[i].options[1], entries[i].options[2]);
        printf("Correct answer: %s\n", entries[i].options[entries[i].answer_idx]);
        printf("\n");
    }

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
