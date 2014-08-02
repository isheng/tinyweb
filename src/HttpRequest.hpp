#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <map>

typedef unsigned char byte;
class HttpRequest
{
private:
	std::vector<byte> *vdata;
	int size;
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> map;

	bool parseRequest(char *data,int length);
public:
	HttpRequest(char *,int);
	~HttpRequest()
	{
		if(vdata!=NULL)
			delete vdata;
	}
	std::vector<byte>* getData()
	{
		return vdata;
	}
	char *GetDataAddr()
	{
		return reinterpret_cast<char *>(&((*vdata)[0]));
	}
	std::string getMethod()
	{
		return method;
	}
	std::string getUri()
	{
		return uri;
	}
	std::string getVersion()
	{
		return version;
	}
	std::string getHeaderContent(std::string key)
	{
		std::map<std::string, std::string>::const_iterator it;
		it=map.find(key);
		if(it!=map.end())
			return it->second;
		else
			return "";
	}
};
bool HttpRequest::parseRequest(char *data,int length)
{
	for(int i=0;i<length;i++)
	{
		if(data[i]=='\n')
		{
			data[i]='\0';
			break;
		}
	}
	std::string firstline(data);
	std::istringstream ins(firstline);
	ins>>method>>uri>>version;
	size=strlen(data);

	std::string requestHeader(data+size+1);
	std::istringstream iin(requestHeader);

	std::string line;
	std::string key,value;
	int pos;
	while(std::getline(iin,line))
	{
		if((pos=(line.find(":")))!=std::string::npos)
		{
			key=line.substr(0,pos);
			value=line.substr(pos+1);
			for (int i = 0; i < value.size(); ++i)
			{
				if(value[i]==' ')
					continue;
				else
				{
					value=value.substr(i);
					break;
				}
			}
			map.insert(make_pair(key,value));
		}
	}

}

HttpRequest::HttpRequest(char* data,int length)
{
	size=length;
	this->parseRequest(data,length);
	vdata=new std::vector<byte>(data+size+1,data+length);
}