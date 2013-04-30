#ifndef MOCK_MESSENGER_H
#define MOCK_MESSENGER_H
//////////////////////////////////////////////////////////////////////////
//  MockMessage.h - A messagenger which process message received on     //
//                  MockChannel                                         //
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
Mock Messenger is a message processor that comming from / going to
Mock Channel.

Public Interface:
=================
MockMessenger mm(ch);
mm(str);       // message processing

Build Process:
==============
Required Files:
- Channel.h, Channel.cpp, User.h, User.cpp, BlockingQueue.h, BlockingQueue.cpp, 
- ClientMessenger.h, ClientMessenger.cpp, Package.h, Package.cpp, Status.h, 
- Status.cpp

Maintenance History:
====================
- Apr 16, 2013 : initial version
*/

#include <string>
#include "../Comm/Channel.h"
#include "../Repository/User.h"
#include "../BlockingQueue/BlockingQueue.h"
#include "ClientMessenger.h"
#include "../Repository/Package.h"

///////////////////////////////////////////////////
// Mock Messenger class
class MockMessenger {
	Channel *ch;

	///////////////////////////////////////////////////
	// a receiver helper class
	class ReceiverHelperThread : public threadBase {
		Channel& ch;
		ClientMessenger mess;
		void run() {
			ch.listen<ClientMessenger>(mess);
		}
	public:
		///////////////////////////////////////////////////
		// constructor
		ReceiverHelperThread(Channel& _ch) : ch(_ch), mess(_ch) {}
	};

	///////////////////////////////////////////////////
	// user login operation processing
	bool userLogin(const std::string& msg) {
		User u;
		// a user-login related message
		if (!u.read(msg)) return false;
		// for logout
		if (!u.isValid()) {	// u is empty at this stage
			if (Status::currentUser().isValid()) {
				std::ostringstream oss;
				oss << "statusmsg [" << Status::currentUser().name <<"] successfully logged out!";
				Status::outQ().enQ(oss.str());
			}
			else {
				Status::outQ().enQ("msg You must log-in first to log-out!");
			}
		}
		else {
			// now contact the server and wait for the response
			Message _msg;
			_msg.fromString(msg);
			ch->send(_msg);
		}
		Status::currentUser() = u;
		return true;
	}

	///////////////////////////////////////////////////
	// process check-in task
	bool checkIn(const std::string& msg) {
		if (msg.substr(0,9)!="check-in ") return false;
		Package p;
		std::vector<std::string> files = p.fromString(msg);
		// send check-in message
		Message _msg;
		_msg.fromString(msg);
		ch->send(_msg);
		return true;
	}

	///////////////////////////////////////////////////
	// process check-in reply
	bool checkInAck(const std::string& msg) {
		if (msg.substr(0,13)!="check-in-ack ") return false;
		Package p;
		std::vector<std::string> files = p.fromString(msg);
		// send check-in message
		Message _msg;
		_msg.fromString(msg);
		ch->send(_msg);
		// send package files
		for (std::string f : files) {
			Message m;
			m.fromFile(f);
			m.fileName() = p.packagePath("", m.fileName());
			if (!m.fileName().empty())
				ch->send(m);
		}
		return true;
	}

	///////////////////////////////////////////////////
	// other instruction such as refresh-reopsitory
	bool directInstruction(const std::string& msg) {
		if (!(msg=="refresh-repository" || msg.substr(0,8)=="extract ")) return false;
		// get package list or extract file
		Message _msg;
		_msg.fromString(msg);
		ch->send(_msg);
		return true;
	}

public:
	ReceiverHelperThread* recv;

	///////////////////////////////////////////////////
	// this will initialize the channel
	MockMessenger() {
		ch = new Channel("Client", Peer("127.0.0.1", 45678));
		recv = new ReceiverHelperThread(*ch);
		recv->start();
	}

	///////////////////////////////////////////////////
	// message processing
	void operator() (const std::string& msg) {
		//Status::outQ().enQ("msg "+ msg);
		if (!userLogin(msg) && !Status::currentUser().isValid()) {
			Status::outQ().enQ("msg You must log-in first to finish this operation!");
			return;
		}
		// now execute instructions
		directInstruction(msg) || checkIn(msg) || checkInAck(msg);
	}

};

#endif