#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/sysmacros.h>

#define BLOCK_SIZE 64

/*Reads the first $bytes_to_read bytes of file in $path and outputs it to out*/
void read_file(char* path, int bytes_to_read, int out)
{
	//Open the file
	int fd = open(path, O_RDONLY);

	//read the file
    int bytes_read;
	char* buffer = malloc(bytes_to_read);
    if ((bytes_read = read(fd, buffer, bytes_to_read)) != 0)
    {
        if (write(out, buffer, bytes_read) < 1)//fd 1 is stdout
        {
            perror("Could not write all bytes to dst");
            exit(1);
        }
        write(out, "\n", 1);
    }	
    free(buffer);
}

/*Reads the cwd and calls readfile*/
void read_dir_files(int bytes_to_read, int out)
{
    struct stat f_stat;

    char d_name[256];
    getcwd(d_name, 255);

    struct dirent* dir;
    DIR* DIR_stream = opendir(d_name);
	char path[256 * 8];

	while ((dir = readdir(DIR_stream)) != NULL)
	{
		if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
		{
            lstat(dir->d_name, &f_stat);
            if (S_ISREG(f_stat.st_mode))
            {
               strcpy(path, d_name);
                if (strcmp(d_name, "/") != 0) 
                    strcat(path, "/");
                strcat(path, dir->d_name);
                read_file(dir->d_name, bytes_to_read, out);
            }
		}
	}
	closedir(DIR_stream);
}

int main(int argc, char *argv[])
{
	char c;
	int bytes_to_read = 0;
	char outFile[256];
    int out;
    bool nOption = false;
	if (argc < 2)
	{
		printf("Correct usage: %s [-n num_bytes][-o outfilename]", argv[0]);
		exit(1);
	}
	
	while ((c = getopt(argc, argv, "n:o:")) != -1)
	{
		switch (c)
		{
		case 'n':
			bytes_to_read = atoi(optarg);
            nOption = true;
			break;
		
		case 'o':
			strcpy(outFile, optarg);
            break;
		
		default:
			break;
		}
	}
	
    if (outFile == NULL)
    {
        out = 1;
    }
    if (!nOption)
    {
        printf("Error, no bytes specified, Exiting\n ");
        exit(1);
    }
    else
    {
        out = open(outFile, O_CREAT | O_RDWR | O_TRUNC);
        if (out == -1)
        {
            perror("Couldnt open file\n");
            exit(1);
        }
             
    }
	read_dir_files(bytes_to_read, out);
	return 0;
}
