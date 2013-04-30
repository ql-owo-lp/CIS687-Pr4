#ifndef REPO_MANAGER_H
#define REPO_MANAGER_H

//////////////////////////////////////////////////////////////////////////
//  RepoManager.h - Manage all packages store on server                 //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//                  Based on version developed by Prof. Fawcett         //
//////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This is a manager that maintains all package stored on server.  It can be
used to search specific package, add new package to server.  It will loaded
when the server starts.

Public Interface:
=================
RepoManager::load();
RepoManager::fillDependencyVer(p);
Package p = RepoManager::findLatest(name, ns);
Package p2 = RepoManager::find(name, ns, v);
Package p3 = RepoManager::find(p);
RepoManager::add(package);

Build Process:
==============
Required Files:
==============
- FIleSystem.h, FileSystem.cpp, Package.h, Locks.h, Locks.cpp, Threads.h, 
- Threads.cpp

Maintenance History:
====================
- Apr 16, 2013 : initial version

*/

#include <unordered_set>
#include <algorithm>
#include <string>
#include <sstream>
#include "Package.h"
#include "../FileSystem/FileSystem.h"
#include "../FileSystem/FileManager.h"
#include "../Threads/Locks.h"
#include "../Threads/Threads.h"

#define SER_REPO_BASEPATH "./ServerRepo"	// the base path of server repository directory

///////////////////////////////////////////////////////////////
// Repository manager
class RepoManager {
	typedef std::unordered_set<Package> repository;	// package list
	static repository _repo;	// the list of all packages
	static bool _loaded;

public:
	typedef repository::iterator iterator;	// iterator of package list

	///////////////////////////////////////////////////////////////
	// load all package list
	static void load() {
		if (_loaded) return;
		_loaded = true;
		_repo.clear();
		FileSystem::Directory::create(SER_REPO_BASEPATH);	// create repository, if not exist
		//list all meta files under the root directory
		std::vector<std::string> metadata_files = FileSystem::Directory::getFiles(SER_REPO_BASEPATH, "*.metadata.xml");
		sout << locker << " "<< metadata_files.size() << " packages are found in server repository.\n" << unlocker;
		//load every package into package list
		for (std::string f : metadata_files) {
			std::ostringstream oss;
			oss << SER_REPO_BASEPATH << "/" << f;
			Package p;
			p.read(oss.str());
			_repo.insert(p);
		}
	}

	///////////////////////////////////////////////////////////////
	// is current check-in open
	static void fillDependencyVer(Package& p) {
		p.checkin_open = false;
		// check if all of the dependencies exist
		for (auto it=p.dependencies.begin(); it!=p.dependencies.end(); it++) {
			Package d;
			if (it->first) {	// if dependency's version is not zero
				if ((d = find(it->second, p.ns, it->first)).empty() || d.checkin_open)
					p.checkin_open = true;
				continue;
			}
			if ((d = findLatest(it->second, p.ns)).empty()) {
				p.checkin_open = true; // mark as check-in open
				it->first = 0;
			}
			else {
				it->first = d.version;	// found!
				// current package cannot be closed if its dependency is not closed
				p.checkin_open = p.checkin_open || d.checkin_open;
			}
		}
	}

	///////////////////////////////////////////////////////////////
	// find the latest version of current package, return empty package when not found
	static Package findLatest(const std::string& _name, const std::string& _ns) {
		Package p;
		std::for_each(_repo.begin(), _repo.end(), 
			[&p, &_name, &_ns] (const Package& _p) {
				if (_p.name == _name && _p.ns == _ns && _p.version > p.version) p = _p;
		});
		return p;
	}

	///////////////////////////////////////////////////////////////
	// find the package with exact name / ns / version
	static Package find(const std::string& _name, const std::string& _ns, const size_t _v) {
		Package p(_name, _ns, _v);
		return find(p);
	}

	///////////////////////////////////////////////////////////////
	// find specific package
	static inline Package find(const Package& p) {
		auto it = _repo.find(p);
		if (it != _repo.end())
			return *it;
		else
			return Package();
	}

	///////////////////////////////////////////////////////////////
	// add when this package does not exist
	static inline void add(const Package& p) {
		if (!p.empty() && find(p).empty())
			_repo.insert(p);
	}

	///////////////////////////////////////////////////////////////
	// first itertor
	static inline iterator begin() {
		return _repo.begin();
	}

	///////////////////////////////////////////////////////////////
	// last iterator
	static inline iterator end() {
		return _repo.end();
	}

	///////////////////////////////////////////////////////////////
	// packag count
	static inline size_t size() {
		return _repo.size();
	}
};

// declare static member
RepoManager::repository RepoManager::_repo;
bool RepoManager::_loaded = false;

#endif