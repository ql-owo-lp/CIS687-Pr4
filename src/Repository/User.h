#ifndef USER_H
#define USER_H
//////////////////////////////////////////////////////////////////////////
//  User.h   -   A data struct defines user information                 //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This package checks whether the username is valid. If the input is a valid user,
compare the username and its password.

Public Interface:
=================
User u;
bool isvalid = u.isValid();                        // is current user a valid user
bool ismatch = u.read(const std::string& cmd);     // read message command

Build Process:
==============
Required Files:
==============
- none

Maintenance History:
====================
- Apr 16, 2013 : initial version

*/

#include <string>

#define LOGIN_STR "login %s %s"	// user login string format
#define LOGOUT_STR "logout"	// user logout string format

	///////////////////////////////////////////////////////////////
// user data
struct User {
	std::string name;	// user name
	std::string passwd;	// user password

	///////////////////////////////////////////////////////////////
	// is current user a valid user
	// if it is not, means currently there is no user loged-in
	bool isValid() const {
		return !name.empty() && !passwd.empty();
	}

	///////////////////////////////////////////////////////////////
	// read message command
	// return whether the command matches
	bool read(const std::string& cmd) {
		char uname[256], pwd[256];
		if (cmd == LOGOUT_STR) {
			name = passwd = "";
			return true;
		}
		else if (sscanf_s(cmd.c_str(), LOGIN_STR, uname, 256, pwd, 256) == 2) {
			name = uname;
			passwd = pwd;
			return true;
		}
		return false;	// invalid format
	}

	///////////////////////////////////////////////////////////////
	// compare username and password
	bool operator==(const User& u) const {
		return name==u.name && passwd==u.passwd;
	}
};

///////////////////////////////////////////////////////////////
// An extension to std namespace
namespace std {
	///////////////////////////////////////////////////////////////
	// A hash function for User struct
	template <>
	struct hash<User> {
		size_t operator()(const User& u) const {
			return std::hash<std::string>()(u.name +" "+ u.passwd);
		}
	};
}

#endif