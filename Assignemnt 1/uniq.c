#include "types.h"
#include "user.h"
#include "stat.h"

#define BUFFSIZE 1024
#define LINEREAD 5012

int PREFIX_LINES = 0;		//Count and group prefix lines
int DUPLICATE_LINES = 0;	//Print only duplicate lights
int IGNORE_CASE = 0;		//Ignore cases		

char buffer[BUFFSIZE];		//Used to read
char buf[LINEREAD];		//For everything else
int builder = 0;		//For incrementing buffer
int flag = 1;			//Used to keep tabs of how many lines found
int i = 0;
int lines[BUFFSIZE];		//Keep track of where each \n is located at
char *temp;			//Holds words temporarily
int occurrences[BUFFSIZE];		//Keeps tracks of the number of occurences for each line.

char * returnLine(int location)
{
	if(location < 0)	//Failsafe
		return "";

	memset(temp, 0, sizeof(temp)*BUFFSIZE);	//Reset the temp char in case of differing sizes

	//Cannot return local variables for some reason, so temp has to be outside.
	int counter = 0;
	for(i = lines[location-1]+1; i < lines[location]-1; i++)		//Grab the section of the line.
	{
		temp[counter] = buf[i];
		counter++;
	}
	//Reset the arrays
	return temp;
}

void PrintOut()
{
	int c;
	for(c = 0; c < flag; c++)
	{
		/* Copy a line into the temp node for comparison */
		char *temp2;
		temp2 = (char *) malloc(8 * sizeof(char));
		strcpy(temp2, returnLine(c));

		if(occurrences[c] > DUPLICATE_LINES)	//If duplicate lines is turned on, then it checks for everything that shows up more than once.
		{
			/* Display prefixes if present */
			if(PREFIX_LINES)		//The prefixes were being kept track of the whole time.  Identify them if this is activated
				printf(1, "%d ", occurrences[c]);
			/* Print out the line itself */
			printf(1, "%s\n", temp2);
		}
		/* Failsafe to reset temp2 */
		memset(temp2, 0, sizeof(temp2));
		free(temp2);
	}
}

int CompareLines(char* left, char* right)
{
	/* strcmp compares cases.  If not turned on, just compare the entire line. */
	if(!IGNORE_CASE)
	{
		return (strcmp(left, right));
	}
	/* If turned on, then we have to compare it char by char. */
	else
	{
		if(strcmp(left, right))
		{
			/* Optimization and failsafe in case two lines are not equal in terms of length */
			if(strlen(left) != strlen(right))
			{
				return 1;
			}

			int d = 0;
			for(d = 0; d < strlen(left); d++)
			{
				if(left[d] != right[d])
				{
					if(left[d] >= 'a' && left[d] <= 'z')		//Switch to uppercase and check again
					{
						if(left[d] - 32 == right[d]);
						else
						{
							return 1;
						}
						//32 is the number to change a char to uppercase or lowercase
					}
					else if(left[d] >= 'A' && left[d] <= 'Z')	//Switch to lowercase and check again
					{
						if(left[d] + 32 == right[d]);
						else
						{						
							return 1;
						}
					}
					else						//It's a number. Show that there's a change
					{
						return 1;
					}
				}
			}
			return 0;	//No changes found.
		}
	}
	return 0;			//Failsafe
}

void uniq(int fd, char *name)
{
	lines[0] = -1;
	int n;
	temp = (char *) malloc(BUFFSIZE*sizeof(char*));

	while((n = read(fd, buffer, sizeof(buffer))) > 0)               //Read any input from pipe or file.  As long as the buffer has input, keep reading.
	{
		//Iterate through the contents we found in the buffer and print them out.
		//In addition, check the contents for any adjacent duplicates.
		for (i = 0; i < n; i++)
		{
			buf[builder++] = buffer[i];
			//Because storing arrays of chars inside an array is impractical and probably unstable, just store the endlines as an alternative.
			if(buffer[i] == '\n')
			{
				occurrences[flag-1] = 0;
				lines[flag] = builder;
				flag++;
				builder++;
			}
		}
	}

	//Overflow or nothing found.
	if( n < 0 )
	{
		printf(1, "Uniq Error:  No values found\n");
		exit();
	}

	int area = 1;
	char *tempor;
	tempor = (char*) malloc(sizeof(char)*BUFFSIZE);
	strcpy(tempor, returnLine(1));

	int c;
	for(c = 2; c <= flag; c++)
	{
		//These lines are needed to artifically extend the string lengths.  Without them, the strcpy function would print out the char variables for some reason.
		//I don't know why it does that.  strcpy doesn't have a print function but it prints them out for some reason.
		char *temp2;
		temp2 = (char *) malloc(sizeof(char)*BUFFSIZE);
		strcpy(temp2, returnLine(c));
		memset(temp, 0, sizeof(temp));		//Reset the temp array in case of varying array lengths

			if(CompareLines(temp2, tempor))		//If Duplicate lines is set, then print only 
			{
				//Update the head temp string for comparison
				occurrences[area]++;
				strcpy(tempor, returnLine(c));
				area = c;
			}
			else
				occurrences[area]++;		//Increment the counter used for the first head detected during comparison.

		//Failsafe reset of temp2
		memset(temp2, 0, sizeof(temp2));
		free(temp2);
	}
	//Print out the results
	PrintOut();

	/* Free up leftover memory */
	free(buffer);
	free(buf);
	free(temp);
	free(tempor);
	free (occurrences);
	free(lines);
}

int main(int argc, char *argv[])
{
  int file_open;
  int i = 0;

//read from standard input if no arguments are passed
  if(argc <= 1){
    uniq(0, "");
    exit();
  }
//Read from a file if one is given on the command line
//If a filename is provided on the command line (i.e., uniq FILE) then uniq should open it,

int file_flag = 1;		//Check if there's a file we can open.

  for(i = 1; i < argc; i++)
  {
	if(!strcmp(argv[i], "-c"))
	{
		//Count and group prefix lines
		PREFIX_LINES = 1;
	}
	else if(!strcmp(argv[i], "-d"))
	{
		//Print only duplicate lines
		DUPLICATE_LINES = 1;

	}
	else if(!strcmp(argv[i], "-i"))
	{
		//Ignore differences in case when comparing
		IGNORE_CASE = 1;
	}
	else
	{
		file_flag = i;		//Copies the index of where the file that could be opened is.
	}
  }

	if((file_open = open(argv[file_flag], 0)) < 0)		//Opens the file
	{
		printf(1, "UniQ Error: cannot open %s\n", argv[file_flag]);
	}
	else
	{
		uniq(file_open, argv[file_flag]);
		close(file_open);
	}

  exit();
}