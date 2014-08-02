#include <cstdio>
#include <cstdlib>
#include <string.h>

#define MAXLINE 4096

int main(int argc, char const *argv[])
{
	char *buf,*p;
	char arg1[MAXLINE],arg2[MAXLINE],content[MAXLINE];
	int n1=0,n2=0;
	// char *cgiargs="124&234";
	// setenv("QUERY_STRING",cgiargs,1);

	if((buf=getenv("QUERY_STRING"))!=NULL)
	{
		p=strchr(buf,'&');
		*p='\0';
		strcpy(arg1,buf);
		strcpy(arg2,p+1);
		n1=atoi(arg1);
		n2=atoi(arg2);
	}

	sprintf(content,"Welcome to add.com:");
	sprintf(content,"%s The Internet addition portal. \r\n<p>",content);
	sprintf(content,"%s The answer is: %d + %d =%d \r\n<p>",content,n1,n2,n1+n2);
	sprintf(content,"%sThanks for visiting!\r\n",content);

	printf("Content-length: %d\r\n",strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s\n", content);
	fflush(stdout);
	return 0;
}