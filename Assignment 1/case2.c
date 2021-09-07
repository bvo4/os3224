#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int priority = 500;
	if(argc > 1)
		priority = atoi(argv[1]);

	const int time = uptime();
	int child = 0;
	int parent = 0;
	int rv;
	int i = 0;
	int j = 0;
	int z = 0;

	for(z = 0; z < 5; z++)
	{
		rv = fork();
			if (rv == 0) {
				ChangeNice(getpid(), priority);

				for(i = 0; i < 5000000; i++);
				child++;
while (wait() > 0);
			}
			else {
				ChangeNice(getpid(), 2);

				for(j = 0; j < 50000000; j++);
				parent++;
				
			}
	}
while (wait() > 0);
	if(rv == 0)
		printf(1, "Finished in %d ticks\n", uptime()-time);

	cps();
	printf(1, "\n");

	exit();
}
