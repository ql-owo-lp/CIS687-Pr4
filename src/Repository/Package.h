#ifndef PACKAGE_H
#define PACKAGE_H
//////////////////////////////////////////////////////////////////////////
//  Package.h  - Defines the metadata of one package.                   //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This is a data struct which defines the metadata of one package, including
its name, namespace, owner, version etc.  It also defines some basic
operations such as reading / wrting package from / to XML.

Public Interface:
=================
Package p;
std::string s = ToString<size_t>(value);                                      // convert E to string
size_t val = ToValue<size_t>(string);                                         // convert string to E
std::vector<std::string> tokens = split(string);                              // split one string into tokens
Package p;
p.fromXML(const std::string& xml);             // load package information from XML
std::string str = p.getUID();                  // get unique id
p.setUID(const std::string& uid);              // read uid and extract information from it
std::string str = p.toXML();                   // write package information to XML
std::string str = p.packagePath(const std::string& base = "", const std::string& filename = "");                // return package path
std::string str = p.fromPackagePath(const std::string& f);        // read package information from the file name of a received file message, return true file name
p.read(const std::string& path);               // get package metadata by package name
p.write(const std::string& base);              // save package metadata to xml file
Package pa = p.merge(Package p);               // merge one package to current package      return empty package when merge fail
bool isempty = p.empty();                      // is current package an empty package
std::vector<std::string> file = p.files(const std::string& base);              // return package files
std::vector<std::string> path = p.dependencyFiles(const std::string& base);    // return path to package files which we are depended on
std::vector<std::string> info = p.fromString(std::string str);                 // read package information from a string (for check-in)  
std::string str = p.toString(std::vector<std::string> package_files);          // convert current package to a string-style representation
p.metadataPath(const std::string& base);                                       // return package metadata path

Build Process:
==============
Required Files:
- FileSystem.h, FileSystem.cpp, FileManager.h, FileManager.cpp, XmlReader.h, 
- XmlReader.cpp, XmlWriter.h, XmlWriter.cpp

Maintenance History:
====================
- Apr 17, 2013 : initial version      
*/

#include <sstream>
#include <string>
#include <vector>
#include "../FileSystem/FileSystem.h"
#include "../FileSystem/FileManager.h"
#include "../XmlReader/XmlReader.h"
#include "../XmlWriter/XmlWriter.h"

///////////////////////////////////////////////////////////////
// convert E to string
template<typename V>
static std::string ToString(V val)
{
	std::ostringstream in;
	in << val;
	return in.str();
}

///////////////////////////////////////////////////////////////
// convert string to E
template<typename V>
static V ToValue(const std::string& str)
{
	std::istringstream in(str);
	V retn;
	in >> retn;
	return retn;
}

///////////////////////////////////////////////////////////////
// split one string into tokens
std::vector<std::string> split(const std::string &str, char delim = '\n') {
	std::vector<std::string> tokens;
	std::istringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim))
		tokens.push_back(item);
	return tokens;
}

#define DEFAULT_NS "default-ns"		// the default namespace when not specific
#define PACKAGE_UID "%[^:]::%[^.].ver%d"	// the unique id of one package

///////////////////////////////////////////////////////////////
// Package meta data 
struct Package {
	std::string owner;	// package owner
	std::string name;	// package name
	std::string ns;		// package namespace
	size_t checkin_open;		// is current package open
	size_t version;	// current package version

	typedef std::pair<size_t, std::string> dependency;
	typedef std::vector<dependency> dependencySet;
	dependencySet dependencies;	// package dependencies

	///////////////////////////////////////////////////////////////
	// constructor, init
	Package() : version(0), checkin_open(false), ns(DEFAULT_NS) {}	// an empty package
	Package(const std::string& _name, const std::string& _ns = DEFAULT_NS, size_t _v = 0)
		: name(_name), version(_v), checkin_open(false), ns(_ns) {}

	///////////////////////////////////////////////////////////////
	// load package information from XML
	void fromXML(const std::string& xml) {
		XmlReader rdr(xml);
		dependencies.clear();
		while (rdr.next()) {
			if (rdr.tag() == "metadata") metadataFromXML(rdr);
			dependencyFromXML(rdr);
		}
	}

	///////////////////////////////////////////////////////////////
	// get unique id
	std::string getUID() const {
		std::ostringstream oss;
		oss << ns <<"::"<< name <<".ver"<< version;
		return oss.str();
	}

	///////////////////////////////////////////////////////////////
	// read uid and extract information from it
	void setUID(const std::string& uid) {
		char _name[256], _ns[256];
		if (sscanf_s(uid.c_str(), PACKAGE_UID, _ns, 256, _name, 256, &version) < 3) {
			name = ns = "";
			return;
		}
		name = _name;
		ns = _ns;
	}

	///////////////////////////////////////////////////////////////
	// write package information to XML
	std::string toXML() const {
		XmlWriter wtr;
		wtr.indent();
		wtr.start("package");
		metadataToXML(wtr);
		dependencyToXML(wtr);
		wtr.end();
		wtr.addDeclaration();
		return wtr.xml();
	}

	///////////////////////////////////////////////////////////////
	// return package path
	std::string packagePath(const std::string& base = "", const std::string& filename = "") const {
		std::ostringstream oss;
		oss << base << "/" << ns << "/" << name << "/" << version;
		if (!filename.empty())
			oss << "/" << filename;
		return oss.str();
	}

	///////////////////////////////////////////////////////////////
	// read package information from the file name of a received file message, return true file name
	std::string fromPackagePath(const std::string& f) {
		char _ns[256], _name[256], _file[256];
		if (sscanf_s(f.c_str(), "/%[^/]/%[^/]/%d/%[^/]", _ns, 256, _name, 256, &version, _file, 256) < 4) return "";
		ns = _ns;
		name = _name;
		return std::string(_file);
	}
	///////////////////////////////////////////////////////////////
	// get package metadata by package name
	void read(const std::string& path) {
		// read package metadata
		std::string metadata = FileManager::read(path);
		fromXML(metadata);
	}

	///////////////////////////////////////////////////////////////
	// save package metadata to xml file
	void write(const std::string& base) const {
		FileSystem::Directory::create(base);
		std::string path(metadataPath(base));	// path to metadata
		FileManager::save(path, toXML());
	}

	///////////////////////////////////////////////////////////////
	// merge one package to current package
	// return empty package when merge fail
	Package merge(Package p) {
		if (empty()) {
			p.version = 1;	// initial version
			return p;
		}
		else if (name!=p.name || ns!=p.ns || owner!=p.owner)
			return Package();
		if (!p.version)
			p.version = version+1;	// version up
		return p;
	}

	///////////////////////////////////////////////////////////////
	// is current package an empty package
	bool empty() const {
		return name.empty() || ns.empty();
	}

	///////////////////////////////////////////////////////////////
	// return package files
	std::vector<std::string> files(const std::string& base) const {
		std::string path(packagePath(base));
		std::vector<std::string> h_files = FileSystem::Directory::getFiles(path, "*.h"),
			cpp_files = FileSystem::Directory::getFiles(path, "*.cpp");
		h_files.insert(h_files.end(), cpp_files.begin(), cpp_files.end());
		for (size_t i=0; i<h_files.size(); i++)
			h_files[i] = path +"/"+ h_files[i];
		return h_files;
	}

	///////////////////////////////////////////////////////////////
	// return path to package files which we are depended on
	std::vector<std::string> dependencyFiles(const std::string& base) const {
		std::vector<std::string> files;
		for (dependency d : dependencies) {
			Package p(d.second, ns, d.first);
			std::vector<std::string> _f = p.files(base);
			files.insert(files.end(), _f.begin(), _f.end());
		}
		return files;
	}

	///////////////////////////////////////////////////////////////
	// read package information from a string (for check-in)
	/*
	FORMAT:
	[check-in] %package_name% %package_ns% %owner% %version%
	%package_dependency% "%dependency_version%	(each dependency takes one line)
	package files		(read until this line)
	%package_files%		(each package file takes one line)
	*/
	std::vector<std::string> fromString(std::string str) {
		char _name[256], _ns[256], _owner[256];
		if (str.substr(0,9)=="check-in ") str = str.substr(9);
		else if (str.substr(0,13)=="check-in-ack ") str = str.substr(13);
		std::vector<std::string> lines = split(str), packfiles;
		if (lines.size() < 2 || sscanf_s(lines[0].c_str(), "%s %s %s %d", _name, 256, _ns, 256, _owner, 256, &version)<4) return packfiles;
		name = _name;
		ns = _ns;
		owner = _owner;
		checkin_open = false;
		bool isPackFile = false;
		for (size_t i=1, ver=0; i<lines.size(); i++) {
			if (lines[i]=="package files") {
				isPackFile = true;
			}
			else if (isPackFile) {
				packfiles.push_back(lines[i]);
			}
			else {
				sscanf_s(lines[i].c_str(), "%s %d", _name, 256, &ver);
				checkin_open = checkin_open || !ver;
				// dependency version will be marked as zero, assume it does not exist
				dependencies.push_back(dependency(ver, std::string(_name)));
			}
		}
		return packfiles;
	}

	///////////////////////////////////////////////////////////////
	// convert current package to a string-style representation
	std::string toString(std::vector<std::string> package_files) {
		std::ostringstream oss;
		oss << name <<" "<< ns <<" "<< owner <<" "<< version;
		for (dependency d : dependencies) {
			oss << "\n" << d.second <<" "<< d.first;
		}
		if (package_files.size()>0)
			oss << "\npackage files";
		for (std::string f : package_files)
			oss << "\n" << f;
		return oss.str();
	}

	///////////////////////////////////////////////////////////////
	// override operator to support compare
	bool operator==(const Package& p) const {
		return name==p.name && ns==p.ns && version==p.version;
	}

	///////////////////////////////////////////////////////////////
	// return package metadata path
	std::string metadataPath(const std::string& base) const {
		std::ostringstream oss;
		oss << base << "/" << ns << "." << name << ".v" << version <<".metadata.xml";
		return oss.str();
	}

private:
	///////////////////////////////////////////////////////////////
	// write dependency to xml writer
	void dependencyToXML(XmlWriter& wtr) const {
		for (dependency depen : dependencies) {
			wtr.start("dependency");
			wtr.addAttribute("version", ToString<size_t>(depen.first));
			wtr.addAttribute("name", depen.second);
			wtr.end();
		}
	}

	///////////////////////////////////////////////////////////////
	// read dependency from xml reader
	void dependencyFromXML(XmlReader& rdr) {
		if (rdr.tag() == "dependency") {
			XmlReader::attribElems attri = rdr.attributes();
			dependency d;
			for (auto val : attri) {
				if (val.first == "name") d.second = val.second;
				else if (val.first == "version") d.first = ToValue<size_t>(val.second);
			}
			dependencies.push_back(d);
		}
	}

	///////////////////////////////////////////////////////////////
	// save metadata to xml
	void metadataToXML(XmlWriter& wtr) const {
		wtr.start("metadata");
		wtr.addAttribute("name", name);
		wtr.addAttribute("owner", owner);
		wtr.addAttribute("version", ToString<size_t>(version));
		wtr.addAttribute("checkin-open", ToString<size_t>(checkin_open));
		wtr.addAttribute("ns", ns);
		wtr.end();
	}

	///////////////////////////////////////////////////////////////
	// read metadata from xml
	void metadataFromXML(XmlReader& rdr) {
		XmlReader::attribElems attri = rdr.attributes();
		for (auto it = attri.begin(); it != attri.end(); it++) {
			if (it->first=="owner") owner = it->second;
			else if (it->first=="version") version = ToValue<size_t>(it->second);
			else if (it->first=="checkin-open") checkin_open = ToValue<size_t>(it->second);
			else if (it->first=="name") name = it->second;
			else if (it->first=="ns") ns = it->second;
		}
	}

};

///////////////////////////////////////////////////////////////
// An extension to std namespace
namespace std {
	///////////////////////////////////////////////////////////////
	// A hash function for User struct
	template <>
	struct hash<Package> {
		size_t operator()(const Package& p) const {
			return std::hash<std::string>()(p.getUID());
		}
	};
}

#endif