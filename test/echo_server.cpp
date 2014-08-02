#include "Sockethttp.hpp"
#include <iostream>
#include <stdio.h>
#include "rio.hpp"

using namespace RIO;

typedef struct 
{
	int maxfd;
	fd_set read_set;
	fd_set readyset;
	int nready;
	int maxi;
	int clientfd[FD_SETSIZE];
	rio_t clientrio[FD_SETSIZE];
}pool;
int byte_cnt=0;

void init_pool(int listenfd,pool *p)
{
	int i;
	p->maxi=-1;
	for(i=0;i<FD_SETSIZE;i++)
		p->clientfd[i]=-1;

	p->maxfd=listenfd;
	FD_ZERO(&p->read_set);
	FD_SET(listenfd,&p->read_set);
}

void add_client(int connfd,pool *p)
{
	int i;
	p->nready--;
	for(i=0;i<FD_SETSIZE;i++)
	{
		if(p->clientfd[i]<0)
		{
			p->clientfd[i]=connfd;
			rio_readinitb(&p->clientrio[i],connfd);
			FD_SET(connfd,&p->read_set);
			if(connfd>p->maxfd)
				p->maxfd=connfd;
			if(i>p->maxfd)
				p->maxfd=i;
			break;
		}
	}
	if(i==FD_SETSIZE)
		perror("add_client error:");
}

void check_clients(pool *p)
{
	int connfd,n;
	char buf[1024];
	rio_t rio;

	for (int i = 0; (i<=(p->maxi)) &&(p->nready>0); ++i)
	{
		connfd=p->clientfd[i];
		rio=p->clientrio[i];

		if((connfd>0)&&(FD_ISSET(connfd,&p->readyset)))
		{
			p->nready--;
			if((n=rio_readlineb(&rio,buf,1024))!=0)
			{
				byte_cnt+=n;
				printf("Server received %d (%d total) bytes on fd %d\n", n,byte_cnt,connfd);
				rio_writen(connfd,buf,n);
			}
			else
			{
				close(connfd);
				FD_CLR(connfd,&p->read_set);
				p->clientfd[i]=-1;
			}
		}
	}
}


void echo(int& connfd)
{
	ssize_t n;
	char buf[4096];
	rio_t rio;

	RIO::rio_readinitb(&rio,connfd);
	while((n=RIO::rio_readlineb(&rio,buf,4096))>0)
	{
		std::cout<<"server receied "<<n<<"bytes   ->";
		printf("%s\n", buf);
		RIO::rio_writen(connfd,buf,n);
	}
}
void command(void)
{
	char buf[1024];
	if(!fgets(buf,1024,stdin))
		exit(0);
	printf("%s",buf);
}

int main()
{

	int listenfd,connfd,clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char * haddrp;
	Sockethttp httpfd;
	static pool p;
	listenfd=httpfd.open_listenfd(6666);

	init_pool(listenfd,&p);

	
	while(true)
	{
		std::cout<<"server start ..."<<std::endl;
		p.readyset=p.read_set;
		clientlen=sizeof(clientaddr);
		p.nready=select(listenfd+1,&p.readyset,NULL,NULL,NULL);
		// if(FD_ISSET(STDIN_FILENO,&readyset))
		// 	command();
		if(FD_ISSET(listenfd,&p.readyset))
		{
			connfd=accept(listenfd,(struct sockaddr *)&clientaddr,(socklen_t *)&clientlen);
			hp=gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr,sizeof(clientaddr.sin_addr.s_addr),AF_INET);
			haddrp=inet_ntoa(clientaddr.sin_addr);
			printf("server connected to %s (%s) \n",hp->h_name,haddrp);
			add_client(connfd,&p);
			// echo(connfd);
			close(connfd);
		}
		check_clients(&p);
	}
	return 0;
}