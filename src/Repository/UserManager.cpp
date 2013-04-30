
//////////////////////////////////////////////////////////////////////////
//  UserManager.cpp - Test UserManager class                            //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////


#ifdef TEST_USER_MANAGER

#include <iostream>
#include "UserManager.h"

//----< test stub >--------------------------------------------
void main() {
	UserManager::load();
	User u;
	u.name = "admin";
	u.passwd = "nimda";
	std::cout << "\n find User? " << UserManager::findUser(u);
	std::cout << "\n\n";
}
#endif