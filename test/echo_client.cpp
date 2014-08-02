#include "Sockethttp.hpp"
#include <string>
#include <stdio.h>
#include "rio.hpp"
int main(int argc, char const *argv[])
{
	std::string hosts="127.0.0.1";
	char *host=const_cast<char *>(hosts.c_str());
	// char *host="127.0.0.1";
	int port=9999;
	char buf[1024];
	Sockethttp shttp;
	shttp.open_clientfd(host,port);

	rio_t rio;
	RIO::rio_readinitb(&rio,shttp.GetSocket());

	while(fgets(buf,1024,stdin)!=NULL)
	{
		RIO::rio_writen(shttp.GetSocket(),buf,1024);
		RIO::rio_readlineb(&rio,buf,1024);
		fputs(buf,stdout);
	}
	
	return 0;
}