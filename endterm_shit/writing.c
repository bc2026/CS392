#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main()
{
	int fd = open("a.txt", O_RDWR); // low-level = kernel buffering
	int fd2 = open("newfile.txt", O_RDWR);
	FILE *f = fopen("b.txt", "w"); // high-level I/O
	// write(fd, "data\0", 0);

	char *string = "the quick brown fox jumped over the lazy dog";
	size_t n_bytes_to_write = sizeof(string) / sizeof(string[0]);

	// printf("%d\n", sizeof(error[0]));
	//  write(fd, string, 4); // write returns bytes writ
	// fwrite(4, 4, 4, f);
	// fclose(f);
	close(fd);

	// close(fd2);
	return 0;
}
