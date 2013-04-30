
//////////////////////////////////////////////////////////////////////////
//	FileManager.cpp - to test FileManager package						//
//																		//
//	ver 0.1																//
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////

#ifdef TEST_FILEMANAGER

#include "FileManager.h"

//----< test stub >--------------------------------------------
int main()
{
	std::cout << "\n\n Now test FileManager";
	FileManager::save("./test.txt", "SAVE SUCCEED!");
	std::string content = FileManager::read("./test.txt");
	std::cout << "\n content "<< content;
	std::cout << "\n\n Test Finished! \n";
}
#endif