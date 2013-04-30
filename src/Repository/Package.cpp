
//////////////////////////////////////////////////////////////////////////
//  Package.cpp -   Test Package class                                  //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////

#ifdef TEST_PACKAGE

#include <iostream>
#include "Package.h"

//----< test stub >--------------------------------------------
void main() {
	Package p;
	std::string s = ToString<size_t>(2);                                      // convert E to string
	std::cout << "\n Output1: "<< s;
	size_t val = ToValue<size_t>("2");                                         // convert string to E
	std::cout << "\n Output2: "<< val;

	std::vector<std::string> tokens = split("Line1\nLine2");                              // split one string into tokens
	std::cout << "\n Output: "<< tokens.size();

	p.fromXML("sample.package.xml");             // load package information from XML
	std::cout << "\n Package GUID: "<< p.getUID();
	std::cout << "\n Package XML: "<< p.toXML();                   // write package information to XML
	std::cout << "\n Package Path: "<< p.packagePath();                // return package path
	std::cout << "\n Package PackagePath: "<< p.fromPackagePath("File");        // read package information from the file name of a received file message, return true file name
	p.read("");               // get package metadata by package name
	p.write("");              // save package metadata to xml file
	Package pa = p.merge(Package());               // merge one package to current package      return empty package when merge fail
	bool isempty = p.empty();                      // is current package an empty package
	std::cout << "\n Package is empty? " << isempty;

	std::cout << "\n Package toString " << p.toString(std::vector<std::string>());          // convert current package to a string-style representation
	std::cout << "\n Package metadata path " << p.metadataPath("");                                       // return package metadata path
}

#endif