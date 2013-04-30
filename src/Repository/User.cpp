
//////////////////////////////////////////////////////////////////////////
//  User.cpp    -   Test User class                                     //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////

#ifdef TEST_USER

#include <iostream>
#include "User.h"

//----< test stub >--------------------------------------------
void main() {
	User u;
	std::cout << "\n Is valid: "<< u.isValid();
	u.name = "uname";
	u.passwd = "pwd";
	User u2;
	u2.read("login uname pwd");
	std::cout << "\n Compare: "<< (u==u2);
}

#endif
