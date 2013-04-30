#ifndef MOCKCHANNEL_H
#define MOCKCHANNEL_H
//////////////////////////////////////////////////////////////////////////                
//  MockChannel.h - A bridge that connects C# GUI and C++ background    //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//                  Based on version developed by Prof. Fawcett         //
//////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This is a managed class which is a bridge connects C# front-end and C++
back-end.  It builds the communication channel by using a managed string type,
convert between String and std::string.

Public Interface:
=================
IChannel chan = IChannel::CreateChannel();
std::string msg = chan.getMessage();
chan.postMessage(msg);
chan.shutDown();

Build Process:
==============
Required Files:
- none

Maintenance History:
====================
- Apr 16, 2013 : initial version
*/

///////////////////////////////////////////////////////////////
// mockchannel namespace
namespace mockChannel {

	using namespace System;

	// define a managed C# string type
	typedef String StrMessage;

	///////////////////////////////////////////////////////////////
	// define IChannel interface
	public ref class IChannel abstract 
	{
	public: 
		///////////////////////////////////////////////////////////////
		// create a new channel
		static IChannel^ CreateChannel();
		///////////////////////////////////////////////////////////////
		// get message from channel
		virtual StrMessage^ getMessage()=0;
		///////////////////////////////////////////////////////////////
		// post message to channel
		virtual void postMessage(StrMessage^ msg)=0;
		///////////////////////////////////////////////////////////////
		// shut down channel
		virtual void shutDown()=0;
	};
}
#endif
