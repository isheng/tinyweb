#include <string>
#include <cstdio>
#include <sstream>
#include <iostream>

class HttpError
{
private:
	int errorcode;
	std::string filepath;
	std::string Body;
	std::string Responese;
	void GeneratErrorHmtl(int code);
	void errorGerate(std::string msg,std::string longmsg);	
public:
	HttpError(int code,std::string path);
	~HttpError(){}

	std::string GetErrorbody()
	{
		return Body;
	}

	std::string GetResponese()
	{
		return Responese;
	}
};

HttpError::HttpError(int code,std::string path):errorcode(code),filepath(path)
{
	GeneratErrorHmtl(code);
}

void HttpError::GeneratErrorHmtl(int errorcode)
{
	switch (errorcode)
	{
		case 404:
		{
			errorGerate("Not Found","CanNot Find this file");
			break;
		}
		case 403:
		{
			errorGerate("Forbidden","You don't have permission to access this file");
			break;
		}
		case 501:
		{
			errorGerate("Not Implemented","Tiny does not Implemented this method");
			break;
		}
		default:
		{
			errorGerate("Error","Web Sever error Or Bad request");
		}
	}
}

void HttpError::errorGerate(std::string msg,std::string longmsg)
{
	std::stringstream ss;
	ss<<"<html><tile>Tiny Error</tile>";
	ss<<"<body bgcolor=\"ffffff\">\r\n";
	ss<<errorcode<<":"<<msg<<"\r\n";
	ss<<"<p>"<<longmsg<<":"<<filepath<<"\r\n";
	ss<<"<hr><em>The tiny web server </em>\r\n";
	ss<<"</body></html>\r\n";
	Body=ss.str();
	
	std::stringstream ss1;

	ss1<<"HTTP/1.0 "<<errorcode<<" "<<msg<<"\r\n";
	ss1<<"Content-type: text/html\r\n";
	ss1<<"Content-length: "<<Body.size()<<"\r\n";

	Responese=ss1.str();
}

// int main(int argc, char const *argv[])
// {
// 	HttpError e(404,"dfsd");
// 	std::cout<<e.GetResponese()<<std::endl;
// 	return 0;
// }