#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buffer [4096];

void tail(int fd, int line)
{
	int n = 0;
	int i = 0;
	int lines = 0;

	int counter = 1;
	
	//Create a temporary file to hold the input.
	int temp  = open("temp", O_CREATE | O_RDWR);
	
	/*******READ FILE*********/
	
	//Read out from the buffer
	while ((n=read(fd, buffer, sizeof(buffer))) > 0)
	{
		//Write into temporary file
		write(temp, buffer, n);
				
		//Check for newlines
		while(i < n)
		{
			if(buffer[i] == '\n')
				lines++;
			
			i++;
		}
	}
	
//Conflicts with pipes
/*
	//File is too small
	if(lines < line)
	{
		printf(1, "ERROR:  NOT ENOUGH LINES\n");
		// delete temp file
		unlink("temp");
		close(temp);
		return;
	}
*/
	//printf(1, "Line: %d\n", line);
	//printf(1, "Lines: %d\n", lines);

	temp = open("temp", 0);
	i = 0;
	counter = 0;
	//int first = 0;

	/*******PRINT FILE*********/

	//Read from the temp file we created
	while((n = read(temp, buffer, sizeof(buffer))) > 0)
	{
		//Take the file stream and read lines
		//Start printing when we reach past our limited number
		//Then, read until end of file
		while(i < n)
		{

			if(counter >= (lines-line))
			{
				//In case no lines are to be taken out, prevent printing out \n at the beginning
				//if(!first)
				//{
				//	printf(1, "%c", buffer[i+1]);
				//	first++;
				//}
				//else
					printf(1, "%c", buffer[i]);
			}
			//Newline
			if(buffer[i]=='\n')
			{
				counter++;
			}
			//printf(1, "Counter: %d\n", counter);


			//printf(1, "LOWER\n");
			//printf(1, "Counter: %d\n", counter);
			i++;
		}
	}

		// delete temp file
		unlink("temp");
		close(temp);
}

int main(int argc, char *argv[])
{
	int read = 10;
	int file_open;
	int i;

	//Default case, print out the last 10 lines
	for(i = 1; i < argc; i++)
	{
		if(*argv[i] == '-')
		{
			argv[i]++;
			//printf(1, "argv: %s\n", argv[i]);
			read = atoi(argv[i]);
		}
		else
		{
			if((file_open = open(argv[i], 0)) < 0)
			{
				printf(1, "Tail:  Cannot open %s\n", argv[i]);
				exit();
			}
		}
	}
	//printf(1, "read: %d\n", read);
	tail(file_open, read);
	close(file_open);
	exit();
}