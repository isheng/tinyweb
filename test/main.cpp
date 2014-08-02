#include "Sockethttp.hpp"


int main(int argc, char const *argv[])
{
	Sockethttp webserver;
	webserver.open_listenfd(9999);
	webserver.process();
	return 0;
}