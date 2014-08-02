#include <boost/unordered_map.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

class MimeTypes
{
	void _createmap();
	std::string filename;
	boost::unordered_map<std::string,std::string> mapType;

public:
	MimeTypes(std::string name="MimeTypes.inc")
	{
		filename=name;
		_createmap();
	}
	~MimeTypes(){}
	std::string getTypes(std::string);
};

void MimeTypes::_createmap()
{
	std::ifstream infile(filename.c_str(),std::ifstream::in);
	std::string line;
	std::string key,value;
	while(std::getline(infile,line))
	{
		std::istringstream ins(line);
		ins>>key>>value;
		mapType.insert(make_pair(key,value));
	}
}
std::string MimeTypes::getTypes(std::string key)
{
	boost::unordered_map<std::string, std::string>::const_iterator it;
	it=mapType.find(key);
	if(it==mapType.end())
		return "text/html";
	else
		return it->second;
}