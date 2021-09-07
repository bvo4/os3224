#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
	int i;
	for(i = 0; i < 5; i++)
	{
		int rv;
		rv = fork();

		if (rv == 0) {
			ChangeNice(getpid(), 20);
			printf(1, "Hello, I'm in the child, my process ID is %d\n",getpid());
		}
		else {
			ChangeNice(getpid(), 2);
			printf(1, "This is the parent process, my process ID is %d and my child is %d\n", getpid(), rv);
		}
	
	}
	wait();
	exit();
}