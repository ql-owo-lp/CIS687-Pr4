#ifndef USER_MANAGER_H
#define USER_MANAGER_H

/////////////////////////////////////////////////////////////////////
// UserManager.h -  A manager that maintains user information      //
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
User manager is responsible for maintaining user database.  It basically
is a user set, which contains all users register on this server.

Public Interface:
=================
UserManager::load();
UserManager::findUser(u);

Build Process:
==============
Required Files:
- XmlReader.h, FileManger.h, XmlReader.cpp, User.h

Maintenance History:
====================
- Apr 23, 2013 : initial version

*/

#include <string>
#include <unordered_set>
#include "User.h"
#include "../XmlReader/XmlReader.h"
#include "../FileSystem/FileManager.h"

#define USR_DAT "./users.database.xml"	// define the user database file

///////////////////////////////////////////////////
// class which manages users
class UserManager {
	static bool _loaded;
	static std::unordered_set<User> _userList;	// list of all users

	///////////////////////////////////////////////////
	// load user information from xml file
	static void fromXML(const std::string& xml) {
		XmlReader rdr(xml);
		_userList.clear();
		while (rdr.next()) {
			if (rdr.tag() == "user") {
				User u;
				XmlReader::attribElems attri = rdr.attributes();
				for (auto it=attri.begin(); it!=attri.end(); it++) {
					if (it->first=="name") u.name=it->second;
					else if (it->first=="passwd") u.passwd=it->second;
				}
				_userList.insert(u);
			}
		}
	}

public:

	///////////////////////////////////////////////////
	// load user list
	static void load() {
		_loaded = true;
		std::string xml = FileManager::read(USR_DAT);
		fromXML(xml);
	}

	///////////////////////////////////////////////////
	// find a match with specific user
	static bool findUser(const User& user) {
		return _userList.find(user) != _userList.end();
	}
};

// initialize current user as a empty-user
std::unordered_set<User> UserManager::_userList;
bool UserManager::_loaded = false;

#endif