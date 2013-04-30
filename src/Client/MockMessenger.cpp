
//////////////////////////////////////////////////////////////////////////
//  MockMessage.cpp - Test Mock Messenger class                         //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////

#ifdef TEST_MOCK_MESSENGER

#include <iostream>
#include "MockMessenger.h"

//----< test stub >--------------------------------------------
void main() {
	Channel ch("Channel", Peer(8081, "127.0.0.1", 8080));
	MockMessenger mess(ch);
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