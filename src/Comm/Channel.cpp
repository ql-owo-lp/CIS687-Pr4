/////////////////////////////////////////////////////////////////////
// Channel.cpp -  Test Channel class                               //
// ver 1.0                                                         //
// Language:      Visual C++, 2011                                 //
// Platform:      Studio 1558, Windows 7 Pro SP1                   //
// Application:   CIS 687 / Project 3, Sp13                        //
// Author:        Kevin Wang, Syracuse University                  //
//                xwang166@syr.edu                                 //
/////////////////////////////////////////////////////////////////////

#ifdef TEST_CHANNEL

#include <string>
#include <iostream>
#include <sstream>
#include "Channel.h"

/////////////////////////////////////////////////////////////////////
// Messenger class, used to processing message
class Messenger {
	typedef std::pair<Peer, Message> MsgPair;
	Channel& ch;	// bind current channel

	///////////////////////////////////////////////////
	// save message content to binary file
	std::string saveBinary(Message& m) {
		::CreateDirectory(L"ReceivedFiles", NULL);	// save files to specific directory
		std::string path("ReceivedFiles/"+ m.fileName());
		std::ofstream f(path, std::ios::out | std::ios::binary);
		m.to(f);
		f.close();
		return path;
	}

	///////////////////////////////////////////////////
	// print string on the screen
	std::string saveString(Message& m) {
		std::ostringstream s;
		m.to(s);
		return s.str();
	}

	///////////////////////////////////////////////////
	// conduct specific task based on message content
	void conductTask(MsgPair msg) {
		std::string instr = saveString(msg.second);
		// we have two types of instructions here
		if (instr=="Task#1") {
			ch.log("Instruction received! Calculation the ultimate answer to the universe..");
			::Sleep(3000);	// just sleep for 3 sec
			ch.log("Calculation finished!");
			// send a message back
			Message m;
			m.fromString("Task#1 Answer: The ultimate answer to the universe is 42");	// return the ultimate answer to the universe
			ch.send(msg.first, m);
		}
		else if (instr=="Task#2") {
			ch.log("Instruction received! Calculation the ultimate question to the universe..");
			::Sleep(3000);
			ch.log("Calculation finished!");
			Message m;
			m.fromString("Task#2 Answer: The ultimate question to the universe is 'How many roads must a man walk down?'");	// return the final question to the universe
			ch.send(msg.first, m);
		}
	}

public:
	///////////////////////////////////////////////////
	// constructor
	Messenger(Channel& _ch) : ch(_ch) {}

	///////////////////////////////////////////////////
	// operate the message
	void operator() (MsgPair& m) {
		if (m.second.isACK()) {
			if (m.second.isBinary())
				ch.log("Binary message ["+ m.second.fileName() +"] is acknowledged by receiver!");
			else
				ch.log("String message is acknowledged by receiver!");
		}
		else if (m.second.isBinary()) {
			std::string path = saveBinary(m.second);
			ch.log("File ["+ m.second.fileName() +"] is received and saved to ["+ path +"]!");
		}
		else {
			std::string str = saveString(m.second);
			ch.log("String ["+ str +"] is received!");
			conductTask(m);	// see if there is any work to do
		}
	}
};

///////////////////////////////////////////////////
// a receiver helper
class ReceiverHelperThread : public threadBase {
	Channel& ch;
	void run() {
		Messenger mess(ch);	// use default messenger here
		ch.listen<Messenger>(mess);
	}
public:
	ReceiverHelperThread(Channel& _ch) : ch(_ch) {}
};

///////////////////////////////////////////////////
// a sender helper
class SenderHelperThread : public threadBase {
	Channel& ch;
	Message& msg;
	void run() {
		ch.send(msg);
	}
public:
	SenderHelperThread(Channel& _ch, Message& _msg) : ch(_ch), msg(_msg) {}
};

//----< test stub >--------------------------------------------
void main() {
	try
	{
		// this demo will run three senders concurrently, two send file to one same port
		// another sender sends a string to a different port

		ReceiverHelperThread* r[2];
		SenderHelperThread* s[2];
		Channel* ch[2];
		Message m[2];
		Peer* p[2];

		p[0] = new Peer(8080, "127.0.0.1", 8081);
		p[1] = new Peer(8081, "127.0.0.1", 8080);

		//m[0].fromFile("../run.bat");
		m[0].fromString("Test string");
		m[1].fromString("Task#1");

		// initialize all channels
		for (size_t i=0; i<2; i++) {
			std::ostringstream os;
			os << "CH" << i;
			ch[i] = new Channel(os.str(), *p[i]);
			r[i] = new ReceiverHelperThread(*ch[i]);
			s[i] = new SenderHelperThread(*ch[i], m[i]);
			s[i]->start();
			r[i]->start();
		}

		// now start all task concurrently
		for (size_t i=0;i<2;i++) {
			s[i]->join();
			r[i]->join();
		}
	}
	catch(...) {
		sout << "\n\n  Something bad happened to a Channel";
	}
	sout << "\n\n";
	system("pause");
}

#endif