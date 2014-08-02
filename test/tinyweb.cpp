#include "rio.hpp"
#include "Sockethttp.hpp"
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iostream>
#include <sys/mman.h>
#include <string>

#define MAXLINE 4096
using namespace std;

void doit(int);
void read_requesthds(rio_t *);
int parse_uri(char *,char *,char *);
void serve_static(int fd,char *filename,int filesize);
void get_filetype(char* filename,char *filetype);
void serve_dynamic(int fd,char* filename,char *cgiargs);
// void clienterror(int fd,char* cause,char *errnum,char *shortmsg,char *longmsg);
void clienterror(int fd,string cause,string errnum,string shortmsg,string longmsg);


int main(int argc, char const *argv[])
{
	int listenfd,connfd,clientlen;
	struct sockaddr_in clientaddr;
	Sockethttp websock;

	listenfd=websock.open_listenfd(6666);

	while(true) {
		clientlen=sizeof(clientaddr);
		connfd=accept(listenfd,(struct sockaddr *)&clientaddr,
			(socklen_t*)&clientlen);
		doit(connfd);
		close(connfd);
	}
	return 0;
}

void doit(int fd)
{
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
	char filename[MAXLINE],cgiargs[MAXLINE];
	rio_t rio;

	RIO::rio_readinitb(&rio,fd);
	RIO::rio_readlineb(&rio,buf,MAXLINE);
	sscanf(buf,"%s %s %s",method,uri,version);
	if(strcasecmp(method,"GET"))
	{
		clienterror(fd,method,"501","Not Implemented",
			"Tiny does not Implemented this method");
		return;
	}

	read_requesthds(&rio);

	is_static=parse_uri(uri,filename,cgiargs);
	if(stat(filename,&sbuf)<0)
	{
		clienterror(fd,filename,"404","Not found",
			"tiny coundn't find this file");
		return;
	}

	if(is_static)
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			clienterror(fd,filename,"403","Forbidden",
				"tiny couldn't read the file");
			return;
		}
		serve_static(fd,filename,sbuf.st_size);
	}
	else
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			clienterror(fd,filename,"403","Forbidden",
				"tiny couldn't read the file");
			return;
		}
		serve_dynamic(fd,filename,cgiargs);
	}
}

void clienterror(int fd,string cause,string errnum,string shortmsg,string longmsg)
{
	char buf[MAXLINE],body[MAXLINE];

	sprintf(body,"<html><tile>Tiny Error</tile>");
	sprintf(body,"%s <body bgcolor=""ffffff"">\r\n",body);
	sprintf(body,"%s%s:%s\r\n",body,errnum.c_str(),shortmsg.c_str());
	sprintf(body,"%s<p>%s: %s\r\n",body,longmsg.c_str(),cause.c_str());
	sprintf(body,"%s<hr><em>The tiny web server </em>\r\n",body);

	sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum.c_str(),shortmsg.c_str());
	RIO::rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Content-type: text/html\r\n");
	RIO::rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body));
	RIO::rio_writen(fd,buf,strlen(buf));

	RIO::rio_writen(fd,body,strlen(body));
}


void read_requesthds(rio_t *rp)
{
	char buf[MAXLINE];

	RIO::rio_readlineb(rp,buf,MAXLINE);
	while(strcmp(buf,"\r\n"))
	{
		RIO::rio_readlineb(rp,buf,MAXLINE);
		printf("%s\n", buf);
	}
	return;
}

int parse_uri(char *uri,char *filename,char *cgiargs)
{
	char *ptr;

	if(!strstr(uri,"cgi-bin"))
	{
		strcpy(cgiargs,"");
		strcpy(filename,".");
		strcat(filename,uri);
		if(uri[strlen(uri)-1]=='/')
			strcat(filename,"index.html");
		return 1;
	}
	else{
		ptr=index(uri,'?');
		if(ptr)
		{
			strcpy(cgiargs,ptr+1);
			*ptr='\0';
		}
		else
			strcpy(cgiargs,ptr+1);
		strcpy(filename,".");
		strcat(filename,uri);
		return 0;
	}
}

void serve_static(int fd,char *filename,int filesize)
{
	int srcfd;
	char *srcp,filetype[MAXLINE],buf[MAXLINE];

	get_filetype(filename,filetype);
	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer: tiny web server\r\n",buf);
	sprintf(buf,"%sContent-length: %d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type: %s \r\n\r\n",buf,filetype);
	RIO::rio_writen(fd,buf,strlen(buf));

	srcfd=open(filename,O_RDONLY,0);
	srcp=static_cast<char *>(mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0));
	close(srcfd);
	RIO::rio_writen(fd,srcp,filesize);
	munmap(srcp,filesize);
}

void get_filetype(char *filename,char *filetype)
{
	if(strstr(filename,".html"))
		strcpy(filetype,"text/html");
	else if(strstr(filename,".gif"))
		strcpy(filetype,"iamge/gif");
	else if(strstr(filename,".jpg"))
		strcpy(filetype,"image/jpeg");
	else
		strcpy(filetype,"text/plain");
}

void serve_dynamic(int fd,char *filename,char *cgiargs)
{
	char buf[MAXLINE],*emptylist[]={NULL};

	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	RIO::rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Server: Tiny Web Server\r\n");
	RIO::rio_writen(fd,buf,strlen(buf));

	if(fork()==0)
	{
		setenv("QUERY_STRING",cgiargs,1);
		dup2(fd,STDOUT_FILENO);
		execve(filename,emptylist,environ);
	}
	wait(NULL);
}