#ifndef FILEMANAGER_H
#define FILEMANAGER_H

//////////////////////////////////////////////////////////////////////////
//	FileManager.h - Manipulate files, including reading / writing       //
//                  files.       	                                    //
//	ver 0.1																//
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////
/*
Module Operations: 
==================
This is basically a helper class which is an extension to the FileSystem.
It includes a couple of static functions which will read/write file, also,
the parse function will directly initialize the ConfigureParser instance and
parse the files with it.

Public Interface:
=================
bool res = FileManager::save(outpath, content);            // save a string to a file
std::string str = FileManager::read(inpath);               // get string from a file

Maintanence Information:
========================
Required files:
---------------
- FileSystem.h, FileSystem.cpp

Maintenance History:
====================
ver 0.1 : 25 April 13
- first version

*/
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include "FileSystem.h"

///////////////////////////////////////////////////////////////
// FileManager class
class FileManager {
public:

	///////////////////////////////////////////////////////////////
	// save a string to a file

	static bool save(const std::string& outpath, const std::string& str) {
		bool res=false;
		std::ofstream outf(outpath);
		if (res = outf.good()) outf<<str;
		outf.close();
		return res;
	}

	///////////////////////////////////////////////////////////////
	// get string from a file

	static std::string read(const std::string& inpath) {
		std::ifstream inf(inpath);
		std::ostringstream ss;
		if (inf.good()) ss << inf.rdbuf() << std::endl;
		inf.close();
		return ss.str();
	}

};

#endif