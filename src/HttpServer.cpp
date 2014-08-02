#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <queue>
#include <fcntl.h>
#include <boost/scoped_ptr.hpp>

#include "Resourse.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "rio.hpp"

#define MAXLINE 1024
#define MAXLISTENFD 50
typedef struct sockaddr SA;

class HttpServer
{
private:
	int listenfd;
	int port;
	struct sockaddr_in serverAddr;
	struct epoll_event ev,events[MAXLISTENFD];
	int epfd;

	void setNoBlock(int& );
	void AcceptConnecton();
	void closeConnection(int);
	int readClient(int);

public:
	HttpServer(int portnum):port(portnum){}
	~HttpServer(){}

	bool start();
	bool process();
	bool stop();
};

bool HttpServer::start()
{
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket error in start:");
		return false;
	}
	int optval=1;
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int))<0)
	{
		perror("setsockopt error in start");
		return false;
	}
	bzero((char*)&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=INADDR_ANY;
	serverAddr.sin_port=htons((unsigned short)port);

	if(bind(listenfd,(SA *)&serverAddr,(socklen_t)sizeof(serverAddr))<0)
	{
		perror("bind error in start:");
		return false;
	}
	setNoBlock(listenfd);

	if(listen(listenfd,SOMAXCONN)<0)
	{
		perror("listen error in start");
		return false;
	}

	epfd=epoll_create1(0);//create epoll 
	if(epfd==-1)
	{
		perror("epoll_create1 error");
		return false;
	}
	ev.data.fd=listenfd;   //ev just for register
	ev.events=EPOLLIN|EPOLLET; //add the event
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev)==-1)  //register
	{
		perror("epoll_ctl error");
		return false;
	}
	return true;
}

bool HttpServer::process()
{
	while(true)
	{
		int nreadyfd=epoll_wait(epfd,events,50,500);
		if(nreadyfd<0)
		{
			perror("epoll_wait error");
			return false;
		}
		for(int i=0;i<nreadyfd;i++)
		{
			if(events[i].data.fd==listenfd)  //accept a new client
			{
				AcceptConnecton();
			}
			else{                    //already exit client hava the event
				if(events[i].events & EPOLLIN)   //client fd can read
				{
					readClient(events[i].data.fd);
				}
				// else if(events[i].events & EPOLLOUT) //
				// {
				// 	writeClient(events[i].data.fd);
				// }
				else
				{
					closeConnection(events[i].data.fd);
				}
			}
		}
	}
}

void HttpServer::AcceptConnecton()
{
	int connfd;
	struct sockaddr_in clienaddr;
	socklen_t sklenth=sizeof(clienaddr);
	std::cout<<"new client add ..."<<std::endl;
	std::cout<<"add connfd"<<std::endl;
	connfd=accept(listenfd,(SA *)&clienaddr,&sklenth);
	if(connfd<0)
	{
		perror("accept error:");
		exit(1);
	}
    char *str = inet_ntoa(clienaddr.sin_addr);
    int cport=ntohs(clienaddr.sin_port);
    std::cout << "accapt a connection from " << str <<" port: "<<cport<<std::endl;
    setNoBlock(connfd);
    ev.data.fd=connfd;
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
}

void HttpServer::closeConnection(int fd)
{
	if(epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL)<0)
	{
		perror("remove fd from epoll_ctl error:");
	}
	close(fd);
}

void HttpServer::setNoBlock(int& sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int HttpServer::readClient(int fd)
{
	char* databuf=new char[1400];
	ssize_t nbytes;
	nbytes=recv(fd,databuf,1400,0);

	if(nbytes==0)
	{
		std::cout<<"client "<<fd<<" close"<<std::endl;
		closeConnection(fd);
		return 0;
	}
	else if(nbytes<0)
	{
		std::cout<<"read the request error with client "<<fd<<std::endl;
		closeConnection(fd);
		return -1;
	}
	else
	{
		boost::scoped_ptr<HttpRequest> req(new HttpRequest(databuf,nbytes));
		boost::scoped_ptr<HttpResponse> resp(new HttpResponse());
		std::auto_ptr<Resourse> R;
		ResourseHost host("../www","index.html");
		R=host.getResourse(req->getUri());
		if(R->getSatecode()==200)
		{
			resp->addFirstline(req->getVersion(),200,"OK");
			resp->addHeader("Content-type",R->getMineType());
			resp->addHeader("Content-length",R->size());
			resp->addContent(R->getAddr(),R->size());
			RIO::rio_writen(fd,resp->getCharAddr(),resp->getSize());

		}
		// else if(R->getSatecode()==404 && req->getUri().find_last_of(".html"))
		// {

		// }
	}
	delete[] databuf;	
}


int main(int argc, char const *argv[])
{
	HttpServer ht(8888);
	ht.start();
	ht.process();
	return 0;
}

