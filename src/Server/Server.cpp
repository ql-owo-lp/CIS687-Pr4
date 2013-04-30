/////////////////////////////////////////////////////////////////////
// Server.cpp  -  Server service listener.                         //
// ver 1.0                                                         //
// Language:      Visual C++, 2011                                 //
// Platform:      Studio 1558, Windows 7 Pro SP1                   //
// Application:   CIS 687 / Project 4, Sp13                        //
// Author:        Kevin Wang, Syracuse University                  //
//                xwang166@syr.edu                                 //
/////////////////////////////////////////////////////////////////////

#include "../Comm/Channel.h"
#include "ServerMessenger.h"
#include "../Repository/RepoManager.h"
#include "../Repository/UserManager.h"
#include <string>
#include <sstream>

///////////////////////////////////////////////////
// a receiver helper class
class ReceiverHelperThread : public threadBase {
	Channel& ch;
	size_t port;
	void run() {
		ServerMessenger mess(ch);	// use default messenger here
		ch.listen<ServerMessenger>(port, mess);
	}
public:
	///////////////////////////////////////////////////
	// constructor
	ReceiverHelperThread(Channel& _ch, size_t _p) : ch(_ch), port(_p) {}
};

//----< program entry >--------------------------------------------
int main(int argc, char* argv[]) {
	try	{
		// load repository first
		RepoManager::load();
		// load user list
		UserManager::load();

		Channel ch("Repository");
		ReceiverHelperThread* r = new ReceiverHelperThread(ch, 45678);
		r->start();

		while (1) ::Sleep(5000);	// prevent program from quitting
	}
	catch(...) {
		sout << "\n  something bad happened";
	}
	return 0;
}
