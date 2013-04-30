#ifndef SERVER_MESSENGER_H
#define SERVER_MESSENGER_H

/////////////////////////////////////////////////////////////////////
// ServerMessenger.h -  Process received message on server         //
// ver 1.0                                                         //
// Language:      Visual C++, 2011                                 //
// Platform:      Studio 1558, Windows 7 Pro SP1                   //
// Application:   CIS 687 / Project 4, Sp13                        //
// Author:        Kevin Wang, Syracuse University                  //
//                xwang166@syr.edu                                 //
/////////////////////////////////////////////////////////////////////
/*

Module Operations:
==================
Server Messenger is responsible for operating received Message on
server-side.  When a string message is received, it will simply print 
it on the screen.  When a binary message is received, it will save it
on the disk.  It will also conduct tasks based on specific string instructions.

Public Interface:
=================
ServerMessenger m(channel);	// declare a messenger instance
m(message);	// process received message

Build Process:
==============
Required Files:
- Message.h, Channel.h, Channel.cpp, User.h, UserManager.h, Package.h, RepoManager.h

Maintenance History:
====================
- Apr 16, 2013 : initial version

*/

#include <string>
#include <fstream>
#include <sstream>
#include "../Comm/Message.h"
#include "../Comm/Channel.h"
#include "../Repository/User.h"
#include "../Repository/UserManager.h"
#include "../Repository/Package.h"
#include "../Repository/RepoManager.h"

/////////////////////////////////////////////////////////////////////
// Messenger class, used to processing message
class ServerMessenger {
	typedef std::pair<Peer, Message> MsgPair;
	typedef std::pair<Peer, std::string> InstrPair;
	Channel& ch;	// bind current channel

	///////////////////////////////////////////////////
	// save message content to binary file
	std::string saveBinary(Message& m) {
		std::ostringstream oss;
		std::vector<std::string> directories = split(m.fileName(), '/');
		oss << SER_REPO_BASEPATH;
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

	///////////////////////////////////////////////////
	// process user login request
	bool task_userLogin(const InstrPair& inst) {
		User u;
		if (!u.read(inst.second)) return false;
		ch.log("Checking user access..");
		Message m;
		if (UserManager::findUser(u)) {
			m.fromString("statusmsg User login successfully!");
			ch.log("User access of ["+ u.name +"] is approved.");
		}
		else {
			m.fromString("login-fail");
			ch.log("User access of ["+ u.name +"] is denied.");
		}
		ch.send(inst.first, m);
		return true;
	}

	///////////////////////////////////////////////////
	// process repository list request
	bool task_refreshRepo(const InstrPair& inst) {
		if (inst.second!="refresh-repository") return false;
		ch.log("Request of package list received.  Sending repository menu..");
		std::ostringstream oss;
		oss << "remote-packages";
		for (RepoManager::iterator it = RepoManager::begin(); it != RepoManager::end(); it++) {
			oss << "\n" << it->getUID();
		}
		Message m;
		m.fromString(oss.str());
		ch.send(inst.first, m);
		return true;
	}

	///////////////////////////////////////////////////
	// process check-in request
	bool task_checkIn(const InstrPair& inst) {
		Package p;
		std::vector<std::string> files = p.fromString(inst.second);
		if (p.empty()) return false;
		bool isChkInACK = (inst.second.substr(0,9)!="check-in ");
		std::ostringstream oss;
		oss << "Check-in request of [" << p.ns << "::" << p.name 
			<<"] (version "<< p.version <<") is received (from user "
			<<p.owner <<").";
		ch.log(oss.str());
		Package _p = RepoManager::findLatest(p.name, p.ns);
		if (!_p.empty() && p.owner != _p.owner) {
			ch.log("Owner of package ["+ p.name +"] mismatch! ("+ p.owner +" != "+ _p.owner +")");
			Message m;
			m.fromString("msg Owner mismatch! You do not have access to check-in this package!");
			ch.send(inst.first, m);
			return true;
		}
		RepoManager::fillDependencyVer(p);
		p = _p.merge(p);
		if (isChkInACK) {
			ch.log("Package ["+ p.getUID() +"] is now added to repository.");
			// add this package to repository
			RepoManager::add(p);
			// save metadata
			p.write(SER_REPO_BASEPATH);
		}
		else {	// replace header, send this message back
			std::string str = "check-in-ack "+ p.toString(files);
			Message m;
			m.fromString(str);
			ch.send(inst.first, m);
		}
		return true;
	}

	///////////////////////////////////////////////////
	// get message for package extraction
	std::vector<Message> task_extract_msgs(const Package& p, bool extractAll) {
		std::vector<Message> msgs;
		std::vector<std::string> files = p.files(SER_REPO_BASEPATH);
		for (std::string f : files) {
			Message m;
			m.fromFile(f);
			m.fileName() = p.packagePath("", m.fileName());
			if (!m.fileName().empty())
				msgs.push_back(m);
		}
		if (extractAll) {	// append dependency files, notice I do not extract every dependency file recursively
			files = p.dependencyFiles(SER_REPO_BASEPATH);
			for (std::string f : files) {
				ch.log("Extract dependency file ["+ f +"]");
				Message m;
				m.fromFile(f);
				m.fileName() = p.packagePath("", m.fileName());
				if (!m.fileName().empty())
					msgs.push_back(m);
			}
		}
		// we also want to send package metadata back to client
		Message m;
		m.fromFile(p.metadataPath(SER_REPO_BASEPATH));
		m.fileName() = "/"+ m.fileName();	// save the file to root directory of repository
		msgs.push_back(m);
		return msgs;
	}

	///////////////////////////////////////////////////
	// extract files
	bool task_extract(const InstrPair& inst) {
		if (inst.second.substr(0,8)!="extract ") return false;
		// the instructions are constructed as ns::packageName.ver%d
		std::vector<std::string> packages = split(inst.second.substr(8)), _tmp;
		std::vector<Message> msgs;
		bool extractAll = (packages[0] == "all");
		for (size_t i=1; i<packages.size(); i++) {
			ch.log("Extraction of package ["+ packages[i] +"] is requested.");
			Package p;
			p.setUID(packages[i]);
			std::vector<Message> _msg = task_extract_msgs(RepoManager::find(p), extractAll);
			msgs.insert(msgs.end(), _msg.begin(), _msg.end());
		}
		for (Message& m : msgs) {
			ch.log("File ["+ m.fileName() +"] is sent to client.");
			ch.send(inst.first, m);	// send every file
		}
		return true;
	}

	///////////////////////////////////////////////////
	// conduct specific task based on message content
	void conductTask(MsgPair msg) {
		InstrPair inst(msg.first, saveString(msg.second));
		task_userLogin(inst) || task_checkIn(inst) || task_extract(inst) || task_refreshRepo(inst);
	}

public:
	///////////////////////////////////////////////////
	// constructor
	ServerMessenger(Channel& _ch) : ch(_ch) {}

	///////////////////////////////////////////////////
	// operate the message
	void operator() (MsgPair& m) {
		if (m.second.isACK()) {
			if (m.second.isBinary())
				ch.log("Binary message ["+ m.second.fileName() +"] is acknowledged by receiver!");
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

#endif