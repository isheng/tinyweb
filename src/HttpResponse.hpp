#include <string.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>

typedef unsigned char byte;
class HttpResponse
{
	std::vector<byte> vcontent;
	size_t size;
	int wpos;
private:
	std::string version,reason;
	int state;

	template<typename T>
	void append(T data)
	{
		if(size+sizeof(T)>vcontent.size())
			return;
		else
		{
			memcpy(&vcontent[wpos],(byte *)&data,sizeof(data));
			wpos+=sizeof(data);
			size+=sizeof(data);
		}
	}

	bool putline(std::string);
	bool putline(char *,int length);
	template<typename T>
	std::string to_string(T data)
	{
		std::stringstream ss;
		ss<<data;
		return ss.str();
	}

public:
	HttpResponse(int buffernum=4098){
		wpos=0;
		size=0;
		vcontent=std::vector<byte>(buffernum);
	}
	~HttpResponse(){

	}



	char* getCharAddr()
	{
		return reinterpret_cast<char *>(&vcontent[0]);
	}
	size_t getSize()
	{
		return size;
	}
	void addHeader(std::string,std::string);
	void addHeader(std::string,int);
	void addFirstline(std::string _version,int code,std::string _reason);

	void addContent(char *,int length);
};

void HttpResponse::addFirstline(std::string _version,int code,std::string _reason)
{
	std::string Header;
	version=_version;
	state=code;
	reason=_reason;
	Header=version+" "+to_string(code)+" "+reason+"\n";
	putline(Header);
}

void HttpResponse::addHeader(std::string type,int content)
{
	putline(type+":"+to_string(content)+"\n");
}

void HttpResponse::addHeader(std::string type,std::string content)
{
	std::string Header=type+": "+content+'\n';
	putline(Header);
}


bool HttpResponse::putline(std::string str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		append(str[i]);
	}
	return true;
}
bool HttpResponse::putline(char *cstr,int length)
{
	for (int i = 0; i < length; ++i)
	{
		append(cstr[i]);
	}
	return true;
}

void HttpResponse::addContent(char *cstr,int length)
{
	putline("\r\n");
	putline(cstr,length);
}


// int main(int argc, char const *argv[])
// {
// 	HttpResponse *resp=new HttpResponse();
// 	resp->addFirstline("HTTP/1.1",200,"OK");
// 	resp->addHeader("Content-type","text/html");
// 	resp->addHeader("Content-length",89);

// 	return 0;
// }