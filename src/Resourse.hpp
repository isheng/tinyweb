#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <cstring>

#include "MimeTypes.h"
typedef unsigned char byte;

class Resourse{
private:
	std::vector<char> data;
	size_t num;
	bool isDir;
	std::string path;
	std::string mimeType;
	int statcode;

public:
	//construtor
	Resourse(){}
	~Resourse(){}
	char* getAddr()
	{
		return (char *)(&data[0]);
	}
	size_t size()
	{
		return num;
	}
	void setSize(size_t length)
	{
		num=length;
	}
	std::vector<char>& getData()
	{
		return data;
	}
	void setData(char* src,size_t len)
	{
		data=std::vector<char>(src,src+len);
	}
	void setData(std::string src)
	{
		data=std::vector<char>(src.begin(),src.end());
		data.push_back('\0');
	}

	void setMimeType(std::string mt) {
		mimeType = mt;
	}
	std::string getMineType()
	{
		return mimeType;
	}
    std::string getLocation() {
        return path;
    }
    void setLocation(std::string uri)
    {
    	path=uri;
    }
	bool isDirectory() {
		return isDir;
	}
	void SetDir(bool is)
	{
		isDir=is;
	}
	int getSatecode()
	{
		return statcode;
	}
	void setStatcode(int code)
	{
		statcode=code;
	}
	
};


class ResourseHost
{
	std::string defaultIndex;
	std::string basepath;
	std::string generateDirList(std::string path,std::vector<char *>& v);
	std::auto_ptr<Resourse> ReadRegularFile(std::string,struct stat&);
	std::auto_ptr<Resourse> ReadDiretory(std::string,struct stat&);

public:
	ResourseHost(std::string path,std::string Index="index.html"):basepath(path),defaultIndex(Index)
	{
	}
	~ResourseHost(){};
	std::auto_ptr<Resourse> getResourse(std::string);
	/* data */
};
std::auto_ptr<Resourse> ResourseHost::getResourse(std::string uri)
{
	struct stat sbuf;
	std::string path=basepath+uri;
	std::auto_ptr<Resourse> res(new Resourse());
	if(stat(path.c_str(),&sbuf)==-1)
	{
		perror("error uri");
		std::cout<<uri<<std::endl;
		res->setStatcode(404);
		return res;
	}
	else if(!(S_ISREG(sbuf.st_mode)|S_ISDIR(sbuf.st_mode))|| !(S_IRUSR & sbuf.st_mode))
	{
		perror("403 forbidden:cannot to read");
		res->setStatcode(403);
		return res;
	}
	else if(S_ISREG(sbuf.st_mode)) //regular file;
	{
		return ReadRegularFile(path,sbuf);
	}
	else
	{
		return ReadDiretory(path,sbuf);
	}
	
}

std::auto_ptr<Resourse> ResourseHost::ReadRegularFile(std::string filepath,struct stat& sbuf)
{
	size_t len=sbuf.st_size;
	MimeTypes tys;
	std::auto_ptr<Resourse> res(new Resourse());
	res->setSize(len);
	res->SetDir(false);
	res->setLocation(filepath);
	res->setStatcode(200);

	int pos=filepath.find_last_of('.');
	std::string tmp=filepath.substr(pos);
	res->setMimeType(tys.getTypes(tmp));

	std::ifstream infile;
	infile.open(filepath.c_str(), std::ifstream::binary);
	char *tmpbuf=new char[len];
	infile.read(tmpbuf,len);
	res->setData(tmpbuf,len);
	delete[] tmpbuf;
	infile.close();

	return res;
}

std::auto_ptr<Resourse> ResourseHost::ReadDiretory(std::string filepath,struct stat& sbuf)
{
	if(filepath.empty() || filepath[filepath.length()-1] != '/')
		filepath+= "/";

	DIR* dir;
	struct dirent *ent;
	std::vector<char *> v;
	struct stat newsbuf;
	std::auto_ptr<Resourse> res(new Resourse());

	dir=opendir(filepath.c_str());
	if(dir==NULL)
	{
		res->setStatcode(404);
		return res;
	}

	while((ent=readdir(dir))!=NULL)
	{
		if(!strcasecmp(ent->d_name,defaultIndex.c_str()))
		{
			std::string newFilePath=filepath+std::string(ent->d_name);
			closedir(dir);
			if(stat(newFilePath.c_str(),&newsbuf)!=-1);
			{
				return ReadRegularFile(newFilePath,newsbuf);
			}
			return std::auto_ptr<Resourse>(NULL);
		}
		v.push_back(ent->d_name);
	}
	std::string datastring=generateDirList(filepath,v);
	closedir(dir);
	res->setSize(datastring.size());
	res->SetDir(true);
	res->setLocation(filepath);
	res->setData(datastring);
	res->setMimeType("text/html");
	res->setStatcode(200);

	return res;
}

std::string ResourseHost::generateDirList(std::string path,std::vector<char *>& v)
{
	std::string uri=path;
	int pos=path.find_last_of(basepath);
	if(pos!=std::string::npos)
		uri=path.substr(pos+1);
	

	std::stringstream ret;
	ret << "<html><head><title>" << uri << "</title></head><body>";

	// Page title, displaying the URI of the directory being listed
	ret << "<h1>Index of " << uri << "</h1><hr /><br />";
    
    std::vector<char*>::const_iterator it=v.begin();
    // Add all files and directories to the return
    while(it!=v.end()) {
		// Skip any 'hidden' files (starting with a '.')
		if(*it[0] != '.')
        	ret << "<a href=\"" << uri << *it << "\">" << *it << "</a><br />";
        it++;
	}
	ret << "</body></html>";
    
    return ret.str();
}


// int main(int argc, char const *argv[])
// {
// 	ResourseHost host("www","index1.html");
// 	std::auto_ptr<Resourse> R=host.getResourse("/blog");

// 	if(R.get()!=NULL)
// 	{
// 		std::string str(R->getData().begin(),R->getData().end()-1);
// 		std::cout<<str<<std::endl;
// 		std::cout<<R->getLocation()<<std::endl;
// 	}
// 	return 0;
// }