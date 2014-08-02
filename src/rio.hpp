#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#define RIO_BUFSIZE 8192

typedef struct{
	int  rio_fd; 				/*desciptor for this itenal buf*/
	int rio_cnt; 				/*unread bytes in internal buf*/
	char *rio_bufptr;			/*next unread bytes in internal buf*/
	char rio_buf[RIO_BUFSIZE];  /*internal buffer*/
}rio_t;

namespace RIO {


	ssize_t rio_readn(int fd,void *usrbuf,size_t n);  //return n which is read or -1 when error    ,without buf
	ssize_t rio_writen(int fd,void *usrbuf,size_t n); // write n byte to the fd or error ,without buf

	void rio_readinitb(rio_t *rp,int fd);

	ssize_t rio_read(rio_t *rp,char *usrbuf,size_t n);
	ssize_t rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen);
	ssize_t rio_readnb(rio_t *rp,void *usrbuf,size_t n);

} // RIO

ssize_t RIO::rio_readn(int fd,void * usrbuf,size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=(char *)usrbuf;

	while(nleft>0)
	{
		if((nread=read(fd,bufp,n))<0)
		{
			if(errno==EINTR)
				nread=0;
			else
				return -1;
		}
		else if(nread==0)
			break;
		nleft-=nread;
		bufp+=nread;
	}
	return (n-nleft);
}

ssize_t RIO::rio_writen(int fd,void *usrbuf,size_t n)
{
	int nleft=n;
	int nwrite;
	char *buf=static_cast<char*>(usrbuf);


	while(nleft>0)
	{
		if((nwrite=write(fd,buf,n))<0)
		{
			if(errno==EINTR)
				nleft=0;
			else
				return -1;
		}
		else
		{
			nleft-=nwrite;
			buf+=nwrite;
		}
	}
	return n;
}

void RIO::rio_readinitb(rio_t *rp,int fd)
{
	rp->rio_fd=fd;
	rp->rio_cnt=0;
	rp->rio_bufptr=rp->rio_buf;
}

ssize_t RIO::rio_read(rio_t *rp,char *usrbuf,size_t n)
{
	int cnt;

	while(rp->rio_cnt<=0)
	{
		rp->rio_cnt=read(rp->rio_fd,rp->rio_buf,sizeof(rp->rio_buf));
		if(rp->rio_cnt<0)
		{
			if(errno !=EINTR)
				return -1;
		}
		else if(rp->rio_cnt==0)
			return 0;
		else
			rp->rio_bufptr=rp->rio_buf;
	}

	cnt=n;
	if(rp->rio_cnt<n)
		cnt=rp->rio_cnt;

	memcpy(usrbuf,rp->rio_bufptr,cnt);
	rp->rio_bufptr+=cnt;
	rp->rio_cnt-=cnt;
	return cnt;
}

ssize_t RIO::rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen)
{
	int n,rc;
	char c;
	char *bufp=static_cast<char *>(usrbuf);

	for(n=1;n<maxlen;n++)
	{
		if((rc=rio_read(rp,&c,1))==1)
		{
			*bufp++=c;
			if(c=='\n')
				break;
		}else if(rc==0)
		{
			if(n==1)
				return 0;
			else 
				break;
		}
		else
			return -1;
	}
	*bufp=0;
	return n;
}

ssize_t RIO::rio_readnb(rio_t *rp,void *usrbuf,size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=static_cast<char *>(usrbuf);

	while(nleft>0)
	{
		nread=rio_read(rp,bufp,n);
		if(nread<0)
		{
			if(errno==EINTR)
				nread=0;
			else
				return -1;
		}
		else if(nread==0)
			break;
		nleft-=nread;
		bufp+=nread;
	}
	return (n-nleft);
}