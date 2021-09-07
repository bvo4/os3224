#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	const int time = uptime();
	int rv = fork();

	while((uptime() - time) < 100);

	if (rv == 0) {
		printf(1, "Child finished\n");
	}
	else {
		printf(1, "Parent finished\n");
	}
	wait();
	exit();
}