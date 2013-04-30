#ifndef CLIENT_MESSENGER_H
#define CLIENT_MESSENGER_H

//////////////////////////////////////////////////////////////////////////                
// ClientMessenger.h - Process message received by client               //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////
/*

Module Operations:
==================
ClientMessenger ---------------------------
Execute tasks based on received specific string instructions.  This module
mainly process message received on client-side.  It will pass message to
MockChannel when it is necessary.

Status ------------------------------------
A helper class which makes it easier to share data between threads. It only
maintains data such as current user.

Public Interface:
=================
ClientMessenger cm(channel);
cm(Channel& _ch);           //constructor

Status s;
User u = s.currentUser();                 // return current user
BQueue inq = s.inQ();                     // return in queue
BQueue outq = s.outQ();                   // return out queue

Build Process:
==============
Required Files:
- Channel.h,Channel.cpp, Message.h,Message.cpp, BlockingQueue.h, 
- BlockingQueue.cpp, Status.h,Status.cpp, Package.h, Package.cpp

Maintenance History:
====================
- Apr 16, 2013 : initial version

*/

#include <string>
#include <fstream>
#include <sstream>
#include "../Comm/Channel.h"
#include "../Comm/Message.h"
#include "../BlockingQueue/BlockingQueue.h"
#include "../Repository/Package.h"
#include "../Repository/User.h"

///////////////////////////////////////////////////////////////
// static class
class Status {
	typedef BlockingQueue<std::string> BQueue;

	static User _curUser;	// current logged-in user
	static BQueue _inQ;     
	static BQueue _outQ;
public:
	///////////////////////////////////////////////////////////////
	// return current user
	static User& currentUser() {
		return _curUser;
	}
	///////////////////////////////////////////////////////////////
	// return in queue
	static BQueue& inQ() {
		return _inQ;
	}
	///////////////////////////////////////////////////////////////
	// return out queue
	static BQueue& outQ() {
		return _outQ;	
	}
};

// declare static member
User Status::_curUser;
Status::BQueue Status::_inQ;
Status::BQueue Status::_outQ;

// path to the client repository
#define CLI_REPO_BASEPATH "../ClientRepo"	// don't sure why I have to go to the parent directory..

/////////////////////////////////////////////////////////////////////
// Messenger class, used to processing message
class ClientMessenger {
	typedef std::pair<Peer, Message> MsgPair;
	Channel& ch;	// bind current channel

	///////////////////////////////////////////////////
	// save message content to binary file
	std::string saveBinary(Message& m) {
		std::ostringstream oss;
		std::vector<std::string> directories = split(m.fileName(), '/');
		oss << CLI_REPO_BASEPATH;
		for (size_t i=1; i<directories.size(); i++) {		// the first one is empty, the last one is true file name
			FileSystem::Directory::create(oss.str());
			oss << "/" << directories[i];
		}
		std::ofstream f(oss.str(), std::ios::out | std::ios::binary);
		m.to(f);
		f.close();
		return FileSystem::Path::getFullFileSpec(oss.str());
	}

	///////////////////////////////////////////////////
	// print string on the screen
	std::string saveString(Message& m) {
		std::ostringstream s;
		m.to(s);
		return s.str();
	}

	///////////////////////////////////////////////////////////////
	// login 
	bool task_login(const std::string& instr) {
		if (instr!="login-fail") return false;
		Status::currentUser() = User();	// invalid it
		Status::outQ().enQ("msg User name and password doesn't match!");
		return true;
	}

	///////////////////////////////////////////////////////////////
	// process check-in reply
	bool task_chkInACK(std::string instr) {
		if (instr.substr(0,13)!="check-in-ack ") return false;
		Package p;
		std::vector<std::string> files = p.fromString(instr);
		if (p.checkin_open)		// this is an open check-in, need prompt
			instr = "open-"+ instr;
		Status::outQ().enQ(instr);
		return true;
	}

	///////////////////////////////////////////////////
	// conduct specific task based on message content
	void conductTask(MsgPair msg) {
		std::string instr = saveString(msg.second);
		if (instr.substr(0, 4) == "msg " || instr.substr(0, 10) == "statusmsg "|| instr.substr(0, 16) == "remote-packages\n") {	// pop up message window directly
			Status::outQ().enQ(instr);
		}
		task_login(instr) || task_chkInACK(instr);
	}

public:
	///////////////////////////////////////////////////
	// constructor
	ClientMessenger(Channel& _ch) 
		: ch(_ch) {}

	///////////////////////////////////////////////////
	// operate the message
	void operator() (MsgPair& m) {
		if (m.second.isACK()) {
			if (m.second.isBinary())
				Status::outQ().enQ("statusmsg File ["+ m.second.fileName() +"] is accepted by repository server!");
		}
		else if (m.second.isBinary()) {
			std::string path = saveBinary(m.second);
			Status::outQ().enQ("statusmsg File ["+ path +"] is downloaded.");
		}
		else {
			conductTask(m);	// see if there is any work to do
		}
	}
};

#endif