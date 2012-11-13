#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

void
err_ret(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(1, 0, fmt, ap);
    va_end(ap);
}

/*
* Fatal error related to a system call.
* Print a message and terminate.
*/

void
err_sys(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(1, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
* Fatal error unrelated to a system call.
* Error code passed as explict parameter.
* Print a message and terminate.
*/

void
err_exit(int error, const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
* Fatal error related to a system call.
* Print a message, dump core, and terminate.
*/

void
err_dump(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(1, 0, fmt, ap);
    va_end(ap);
    abort();        /* dump core and terminate */
    exit(1);        /* shouldn't get here */
}
/*
* Nonfatal error unrelated to a system call.
* Print a message and return.
*/

void
err_msg(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
}


/*
* Fatal error unrelated to a system call.
* Print a message and terminate.
*/

void
err_quit(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

void daemonize(const char *cmd)
{
	int i,fd0,fd1,fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	umask(0);

	if(getrlimit(RLIMIT_NOFILE, &rl)<0)
		err_quit("%s: can't get file limit", cmd);

	if((pid=fork())<0)
		err_quit("%s: can't fork", cmd);
	else
		if(pid!=0)
			exit(0);

	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP,&sa, NULL)<0)
		err_quit("%s: cant't ignore SIGHUP");
	if((pid = fork())<0)
		err_quit("%s: cant't fork", cmd);
	else
		if(pid!=0)
			exit(0);
	
	if( chdir("/")<0)
		err_quit("%s: can't change directory to /");

	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i = 0;i<rl.rlim_max;i++)
	{
		printf("i = %d\n",i);
		close(i);
	}

	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(cmd, LOG_CONS, LOG_DAEMON);
	printf("fd0 = %d fd1 = %d fd2 = %d\n",fd0,fd1,fd2);
	if(fd0 != 0 || fd1!=1 || fd2!=2)
	{
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0,fd1,fd2);
		printf("we exit here\n");
		exit(1);
	}
}

