
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "logger.h"





struct timeval start;

void timestamp (void)
{
	struct timeval now;
	gettimeofday(&now, 0);
	struct timeval dt;
	timersub(&now, &start, &dt);
	log_printf("[%d.%03ld] ", dt.tv_sec, dt.tv_usec/1000);
}

void set_shell_back (void)
{
	if (access("/etc/inittab.bak", F_OK))
	{
		int err = system("/usr/bin/awk '{sub(/tty_test/,\"-/bin/sh\");print}' /etc/inittab");
		log_printf("system() %d\n", err);
	} else {
		int err = remove("/etc/inittab");
		log_printf("remove(\"/etc/inittab\") %d\n", err);

		err = rename("/etc/inittab.bak", "/etc/inittab");
		log_printf("rename(\"/etc/inittab.bak\", \"/etc/inittab\") %d\n", err);
	}
}

void log_std_fileno (void)
{
	int flags = fcntl(STDIN_FILENO, F_GETFD, 0);
	log_printf("STDIN_FILENO flags = 0x%X\n", flags);
	flags = fcntl(STDOUT_FILENO, F_GETFD, 0);
	log_printf("STDOUT_FILENO flags = 0x%X\n", flags);
	flags = fcntl(STDERR_FILENO, F_GETFD, 0);
	log_printf("STDERR_FILENO flags = 0x%X\n", flags);
}


int main (int argc, const char* argv[])
{
	gettimeofday(&start, 0);

	log_start("/tty_test.log", LOG_APPEND, 0);

	timestamp();
	log_printf("tty_test START\n");
	log_printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++)
		log_printf("argv[%d] = %s\n", i, argv[i]);
	log_std_fileno();
	int count = write(STDOUT_FILENO, "STDOUT ", 7);
	log_printf("write STDOUT_FILENO %d\n", count);
	count = write(STDERR_FILENO, "STDERR ", 7);
	log_printf("write STDERR_FILENO %d\n", count);
	char buff[100] = {0};
	count = read(STDIN_FILENO, buff, 100);
	log_printf("read STDIN_FILENO %d\n", count);
	for (int i = 0; i < count; i++)
		log_printf("buff[%d] = 0x%X\n", i, buff[i]);
	timestamp();

//TODO: close standard input and output and open /dev/ttyS0 as controlling terminal or as usual file
// or change current control terminal settings

	set_shell_back();

	timestamp();
	log_stop();

	return 0;
}

