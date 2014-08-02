#include "HttpRequest.hpp"
using namespace std;
int main(int argc, char const *argv[])
{

	std::string ss="GET / HTTP/1.1\r\nHost: www.gdufs.edu.cn\r\n";
	char* str=const_cast<char*>(ss.c_str());
	HttpRequest *req=new HttpRequest(str,strlen(str));
	std::cout<<req->getHeaderContent("Host");
	return 0;
}