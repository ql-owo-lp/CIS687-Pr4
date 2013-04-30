/////////////////////////////////////////////////////////////////////
// ServerMessenger.cpp - Test ServerMessenger class                //
// ver 1.0                                                         //
// Language:      Visual C++, 2011                                 //
// Platform:      Studio 1558, Windows 7 Pro SP1                   //
// Application:   CIS 687 / Project 4, Sp13                        //
// Author:        Kevin Wang, Syracuse University                  //
//                xwang166@syr.edu                                 //
/////////////////////////////////////////////////////////////////////

#ifdef TEST_SERVER_MESSENGER

#include <iostream>
#include "../Comm/Message.h"
#include "../Comm/Channel.h"
#include "ServerMessenger.h"

//----< test stub >--------------------------------------------
void main() {
	Channel ch("Channel", Peer(8081, "127.0.0.1", 8080));
	ServerMessenger mess(ch);
	Message m;
	std::cout<<"\n Reading from run.bat..";
	// test binary message here
	m.fromFile("../run.bat");
	std::cout<<"\n Message length: "<< m.length()
		<<"\n Block size: "<< m.blockSize()
		<<"\n Block number: "<< m.size()
		<<"\n Message name: "<< m.fileName();
	mess(m);	// use messenger to process message

	// test string message here
	Message m2;
	std::cout<<"\n Testing string message..";
	m2.fromString("Test string message");
	std::cout<<"\n Message length: "<< m.length()
		<<"\n Block size: "<< m.blockSize()
		<<"\n Block number: "<< m.size()
		<<"\n Message name: "<< m.fileName();
	mess(m2);
	std::cout<<"\n\n";
}
#endif