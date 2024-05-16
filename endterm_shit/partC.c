int main(int argc, char** argv)
{
	char* grep_args[] = {"grep", "cs392", NULL};

	int in = open("cs392.txt", O_RDONLY) // read only
	
	int pipefd[2];
	pid_t pid;


	if(pipe(pipefd) == -1) 
	{
		return -1;
	}

	pid = fork();

	if(pid < 0) { return -1};

	else if(pid == 0) {} // child process
	else {} // parent process
	close(in);
	execvp("grep", grep_args);
}	
