// Bhagawat Chapagain
// I pledge my honor that I have abided by the Stevens Honor System.

#define BLUE "\x1b[34;1m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define RED "\033[0;31m"
#define DEFAULT "\x1b[0m"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h> // Include the signal handling library
volatile sig_atomic_t interrupted = 0;

// Signal handler for SIGINT
void handle_sigint(int sig)
{
    interrupted = 1;
}

void cd();
void lf();
void lp();

typedef enum
{
    CD,         // "cd"
    PWD,        // "pwd"
    LF,         // "lf"
    LP,         // "lp"
    EXIT,       // "exit"
    CMD_UNKNOWN // For any string that is not "a", "b", "c", or "d"
} StringEnum;

// Function to convert string to enum
StringEnum stringToEnum(const char *str)
{
    if (strcmp(str, "cd") == 0)
        return CD;
    if (strcmp(str, "pwd") == 0)
        return PWD;
    if (strcmp(str, "lf") == 0)
        return LF;
    if (strcmp(str, "lp") == 0)
        return LP;
    if (strcmp(str, "exit") == 0)
        return EXIT;
    return CMD_UNKNOWN; // Default case
}

bool is_integer(const char *str)
{
    char *endptr;
    errno = 0; // To distinguish success/failure after the call

    long val = strtol(str, &endptr, 10);

    // Check for various possible errors

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
    {
        perror("strtol");
        return false;
    }

    if (endptr == str)
    {
        // fprintf(stderr, "No digits were found\n");
        return false; // No digits found
    }

    // If we're here, strtol() successfully parsed a number

    if (*endptr != '\0')
    {
        // Additional characters after number
        return false;
    }

    // If you want to check if it fits into an `int`
    if (val > INT_MAX || val < INT_MIN)
    {
        return false; // Value overflows int
    }
    return true; // String is an integer
}

void lf(char *path)
{
    DIR *d = opendir(path);
    if (d == NULL)
        return;

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type != DT_DIR)
        {
            // If the entry is not a directory
            printf("%s\n", dir->d_name); // Print file name in blue, then reset color
        }
        else if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
        {
            // If it is a directory (excluding . and ..)
            printf("%s%s%s\n", BLUE, dir->d_name, DEFAULT); // Print directory name in blue, then reset color
        }
    }
    closedir(d); // Close the directory
}

int compare_pids(const void *a, const void *b)
{
    int pid1 = *(const int *)a;
    int pid2 = *(const int *)b;
    return pid1 - pid2;
}

void lp()
{
    DIR *dr;
    struct dirent *en;
    int *processes = NULL;
    size_t count = 0;
    size_t size = 10;

    dr = opendir("/proc");
    if (!dr)
    {
        perror("/proc");
        exit(EXIT_FAILURE);
    }

    processes = malloc(size*sizeof(int));
    if (!processes)
    {
        fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        closedir(dr);
        exit(EXIT_FAILURE);
    }

    while ((en = readdir(dr)) != NULL)
    {
        if (count == size)
        {
            size *= 2;
            int *temp_processes = realloc(processes, size * sizeof(int));
            if (!temp_processes)
            {
                fprintf(stderr, "Error: realloc() failed. %s.\n", strerror(errno));
                free(processes);
                closedir(dr);
                exit(EXIT_FAILURE);
            }
            processes = temp_processes;
        }

        char *endptr;
        int pid = strtol(en->d_name, &endptr, 10);
        if (*endptr == '\0' && pid > 0)
        {
            processes[count++] = pid;
        }
    }
    closedir(dr);

    qsort(processes, count, sizeof(int), compare_pids);

    for (size_t i = 0; i < count; i++)
    {
        char path_status[256];
        char path_cmdline[256];
        snprintf(path_status, sizeof(path_status), "/proc/%d/status", processes[i]);
        snprintf(path_cmdline, sizeof(path_cmdline), "/proc/%d/cmdline", processes[i]);

        FILE *file_status = fopen(path_status, "r");
        FILE *file_cmdline = fopen(path_cmdline, "r");
        if (!file_status || !file_cmdline)
        {
            perror("Failed to open file");
            if (file_status)
                fclose(file_status);
            if (file_cmdline)
                fclose(file_cmdline);
            continue; // Proceed to the next PID instead of exiting
        }

        char line[1024], user[256] = "N/A", cmd[1024] = "";
        int uid_found = 0;
        while (fgets(line, sizeof(line), file_status) && !uid_found)
        {
            if (strncmp(line, "Uid:", 4) == 0)
            {
                uid_found = 1;
                int uid = atoi(line + 5); // Uid: is 5 characters
                struct passwd *pwd = getpwuid(uid);
                if (pwd != NULL)
                {
                    strncpy(user, pwd->pw_name, sizeof(user) - 1);
                    user[sizeof(user) - 1] = '\0';
                }
                else
                {
                    fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
                    continue;
                }
            }
        }

        size_t cmd_len = fread(cmd, 1, sizeof(cmd) - 1, file_cmdline);
        for (size_t j = 0; j < cmd_len - 1; ++j)
        {
            if (cmd[j] == '\0')
                cmd[j] = ' '; // Replace null separators with spaces
        }
        cmd[cmd_len] = '\0';

        printf("%d %s %s\n", processes[i], user, cmd);

        fclose(file_status);
        fclose(file_cmdline);
    }

    free(processes);
}

void cd(char *arg, int argCount)
{

    if (argCount > 1)
    {
        fprintf(stderr, "Error: Too many arguments to cd.\n");
        return;
    }
    char fullPath[PATH_MAX]; // Buffer to hold the full path

    if (arg == NULL || arg[0] == '\0')
    {
        // Get the HOME environment variable
        char *home = getenv("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "Error: Cannot get HOME environment variable. %s.\n", strerror(errno));
            return;
        }
        if (chdir(home) != 0)
        {
            fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", home, strerror(errno));
        }
        return;
    }

    if (arg[0] == '~')
    {
        // If arg starts with "~", expand it to the user's home directory
        char *home = getenv("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "Error: Cannot get HOME environment variable. %s.\n", strerror(errno));
            return;
        }
        snprintf(fullPath, sizeof(fullPath), "%s%s", home, arg + 1); // Skip over the '~'
    }
    else
    {
        strncpy(fullPath, arg, sizeof(fullPath));
        fullPath[sizeof(fullPath) - 1] = '\0'; // Ensure null-termination
    }

    // Attempt to change directory
    if (chdir(fullPath) != 0)
    {
        fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", fullPath, strerror(errno));
    }
}

int main(int argc, char **argv)
{
    char last_command[100];
    char *cwd; // Pointer to store the current working directory
    struct sigaction sa;

    // Set up the signal handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    //sa.sa_flags = SA_RESTART; // To restart system calls if interrupted

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Initialize the current working directory
    cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Main command loop
    do
    {
        printf("%s[%s]%s> ", BLUE, cwd, DEFAULT);

        if (interrupted) {
            // Print a new line for a clean prompt
            printf("\n");
            // Reset the flag
            interrupted = 0;
            // Clear the stdin buffer if necessary
        
            // Print the shell prompt again
            // Continue with the next iteration of the loop
            continue;
        }
        if (fgets(last_command, sizeof(last_command), stdin) == NULL)
        {
            if (errno == EINTR && interrupted)
            {
                // If fgets was interrupted by SIGINT, clear the error and prompt again
                clearerr(stdin);
                interrupted = 0; // Reset the interrupted flag
                printf("\n");
                continue;        // Skip the rest of the loop and display the prompt again
            }
            else if (feof(stdin))
            {
                // End of input (Ctrl+D)
                printf("\n");
                break;
            }
            else
            {
                // An actual error occurred
                fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
                break;
            }
        }

        // Remove the newline character at the end of the command
        last_command[strcspn(last_command, "\n")] = '\0';

        // Use strtok to split the string by space
        char *tokens[64]; // Arbitrary large size for simplicity
        int tokenCount = 0;
        char *token = strtok(last_command, " ");
        while (token != NULL && tokenCount < 64)
        {
            tokens[tokenCount++] = token;
            token = strtok(NULL, " ");
        }

        StringEnum CMD = stringToEnum(tokens[0]);
        switch (CMD)
        {
        case CD:
            // Pass the first argument and the number of arguments to cd

            if (tokenCount - 1 != 0)
            {
                cd(tokens[1], tokenCount - 1);
            }
            else
            {
                cd("~", tokenCount - 1);
            }
            cwd = getcwd(NULL, 0);
            if (!cwd)
            {
                fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        case PWD:
            // pwd: works
            printf("%s\n", cwd);
            break;
        case LF:
            lf(cwd);
            break;
        case LP:
            lp();
            break;
        case CMD_UNKNOWN:
            // works
            char *command = tokens[0];
            tokens[tokenCount] = NULL; // Ensure arguments are NULL-terminated

            pid_t pid = fork(); // Create a new process
            int status;

            if (pid == -1)
            {
                // If fork() fails, print an error and break
                fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                // Child process
                if (execvp(command, tokens) == -1)
                {
                    fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                    exit(EXIT_FAILURE); // Exit the child process
                }
            }
            else
            {

                if(waitpid(pid, &status, 0) == -1)
                {
                    if(errno == EINTR && interrupted)
                    {
                    clearerr(stdin);
                    interrupted = 0;
                    printf("\n");
                    }
                }
                else
                {
                    fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                }
            }
        }
    } while (strcmp(last_command, "exit") != 0);
    free(cwd); // Free the memory allocated for cwd when exiting the loop
    exit(EXIT_SUCCESS);
}
// <- nice