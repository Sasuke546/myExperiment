#include "apue.h"
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <stdio.h>

#define BUFLEN 128
#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen)
{
	int fd;
	int err = 0;
	if((fd = socket(addr->sa_family, type,0))<0)
		return -1;
	if(bind(fd, addr, alen) <0)
	{
		err = errno;
		close(fd);
		errno = err;
		return -1;
	}
	if(type == SOCK_STREAM || type == SOCK_SEQPACKET)
	{
		if(listen(fd,qlen)<0)
		{
			err = errno;
			close(fd);
			errno = err;
			return -1;
		}
	}
	return fd;
}

void serve(int sockfd)
{
	int clfd;
	FILE *fp;
	char buf[BUFLEN];

	for(;;)
	{
		clfd = accept(sockfd, NULL, NULL);
		printf("accepting!\n");
		if(clfd<0)
		{
			//syslog(LOG_ERR, "ruptimed: accept error: %s",strerror(errno));
			printf("ruptimed: accept error: %s",strerror(errno));
			exit(1);
		}
		if((fp = popen("/usr/bin/uptime","r"))==NULL)
		{
			sprintf(buf,"error: %s\n", strerror(errno));
			send(clfd,buf,strlen(buf),0);
			pclose(fp);
		}
		else
		{
			while(fgets(buf,BUFLEN,fp)!=NULL)
				send(clfd,buf,strlen(buf),0);
			pclose(fp);
		}
		close(clfd);
	}
}

int main(int argc, char *argv[])
{
	struct addrinfo *ailist, *aip;
	struct addrinfo hint;
	int sockfd, err, n;
	char *host;
	
	if(argc!=1)
	{
		printf("usage: ruptimed");
		exit(0);
	}

#ifdef _SC_HOST_NAME_MAX

	n = sysconf(_SC_HOST_NAME_MAX);
	if(n<0)
#endif
		n = HOST_NAME_MAX;

	host = malloc(n);
	if(host == NULL)
	{
		printf("malloc error");
		exit(0);
	}

	if(gethostname(host,n)<0)
	{
		printf("gethostname error");
		exit(0);
	}

	printf("host name: %s\n",host);
//	daemonize("uptimeServer");
//	printf("OK!\n");
	hint.ai_flags = AI_CANONNAME;
	hint.ai_family = 0;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	if((err = getaddrinfo(host, "ruptime", &hint, &ailist))!=0)
	{
		//syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s",gai_strerror(err));
		printf( "ruptimed: getaddrinfo error: %s\n",gai_strerror(err));
		exit(1);
	}

	printf("get addressinfo ok\n");
	for(aip = ailist; aip!=NULL; aip = aip->ai_next)
	{
		if((sockfd = initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN)) >=0 )
		{
			serve(sockfd);
			exit(0);
		}
	}
	exit(1);
}


