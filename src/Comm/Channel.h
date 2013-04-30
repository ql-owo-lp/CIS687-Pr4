#ifndef CHANNEL_H
#define CHANNEL_H

/////////////////////////////////////////////////////////////////////
// Channel.h -    Provides ability to communicate using socket     //
// ver 1.0                                                         //
// Language:      Visual C++, 2011                                 //
// Platform:      Studio 1558, Windows 7 Pro SP1                   //
// Application:   CIS 687 / Project 3, Sp13                        //
// Author:        Kevin Wang, Syracuse University                  //
//                xwang166@syr.edu                                 //
/////////////////////////////////////////////////////////////////////
/*

Module Operations:
==================
Channel package is used to build connection between two peers.  It
consists of one Channel class and one Peer data structure.

Channel:
-------
Channel class will build a channel, send / receive data to / from 
remote client.  Consider it is a layer which sustains the connection
status between two peers, that it will close the connection after sending
or receiveing a message each time.  Or, it will also be able to sustain
the connection when it is set to (typically when there is a "keep-alive"
flag in the message it receives).

Channel class also controls some low-level sending details such as replying
acknowledge message to the sender.  Channel package should hide all the
communication details from high-level classes.

Peer:
-------
Peer data structure is used to store the peer information, typically remote
IP, remote port, local IP, local port.  It can be used to distinguish different
channels by using this pair as ID.

Public Interface:
=================
Peer p;
p.fill(socket);	                         // fill data with socket information
std::string str = p.toString();	         // return this pair as string
std::string remote = p.remoteHost();	 // return remote host string
std::string local = p.localHost();	     // return local host string

Channel ch(p);	                         // create a channel with specific peer
ch.enableACK()=true;	                 // enable ACK on channel
ch.send(p, msg);	                     // send message to specific peer
ch.send(msg);	                         // send message to paired remote peer
ch.listen<Messenger>(port, func);	     // listen to a specific port
ch.listen<Messenger>(f);	             // listen to paired peer port

Build Process:
==============
Required Files:
Sockets.h, Sockets.cpp, Locks.h, Threads.h, BlockingQueue.h, BlockingQueue.cpp, Message.h, HttpWrapper.h

Maintenance History:
====================
- Apr 16, 2013 : initial version

*/

#include <string>
#include <unordered_map>
#include <sstream>
#include <time.h>
#include "../Sockets/Sockets.h"
#include "../Threads/Locks.h"
#include "../Threads/Threads.h"
#include "../BlockingQueue/BlockingQueue.h"
#include "Message.h"
#include "HttpWrapper.h"

/////////////////////////////////////////////////////////////////////
// Peer class
// here we define what is a peer (IP, port, etc.)
struct Peer {
	std::string remote;	// remote's IP address
	size_t lport;	// local port
	size_t rport;	// remote port

	// default constructor
	Peer() : lport(0), rport(0) {}

	// fill data with specific socket
	Peer(Socket& s) {
		fill(s);
	}

	// constructors
	Peer(std::string _remote, size_t _rport) : remote(_remote), rport(_rport), lport(0) {}
	Peer(size_t _lport, std::string _remote, size_t _rport) : remote(_remote), rport(_rport), lport(_lport) {}

	// fill information from socket
	void fill(Socket& s) {
		remote = s.System().getRemoteIP(&s);
		lport = s.System().getLocalPort(&s);
		rport = s.System().getRemotePort(&s);
	}

	// return this pair as string
	std::string toString() {
		std::ostringstream ss;
		ss << "[" << remote << ":" << rport << "](local " << lport <<")";
		return ss.str();
	}

	// return remote host string
	std::string remoteHost() {
		std::ostringstream ss;
		ss << remote << ":" << rport;
		return ss.str();
	}
};

/////////////////////////////////////////////////////////////////////
// Channel class
class Channel {
	typedef std::pair<Peer, Message> MsgPair;
	typedef BlockingQueue<MsgPair> messageQ; // data buffer queue, for a whole message
	// NOTE: only one receive port is support on one single channel!
	messageQ sendQ;		// send buffer queue

	bool _enableACK;	// whether to enable ACK or not
	Peer _remotePeer;	// default remote peer
	std::string channelName;	// channel name
	size_t _listen_port;	// local listening port

	///////////////////////////////////////////////////
	// ClientHandlerThread thread
	// Hiding details from upper layer, thus I define it inside Channel
	class ClientHandlerThread : public tthreadBase {
		Socket s;	// socket
		messageQ& q;	// message queue
		Channel& ch;
		std::unordered_map<std::string, Message> MsgSet;	// transmission name, message

		///////////////////////////////////////////////////
		// process received message
		void processMsg(HttpWrapper& wrapper, const std::string& msgid, Peer p) {
			// if this message is complete
			if (wrapper.isAllMsgArrived() || wrapper.isACK()) {
				if (ch.enableACK() && !wrapper.isACK()) {	// send an ACK for received message
					Message ack(wrapper.fileName());
					ack.isACK() = true;
					// push one empty data block to message
					ack.push(DataBlock());
					p.rport = wrapper.replyPort();
					ch.send(p, ack);
				}
				q.enQ(MsgPair(p, MsgSet[msgid]));
				MsgSet.erase(msgid);
			}
		}

		///////////////////////////////////////////////////
		// read message from socket
		void readMsg(const std::string& header, Peer& p) {
			// use HTTP wrapper to read the header
			HttpWrapper wrapper;
			if (!wrapper.readHeader(header)) {
				ch.log("Mal-formatted header message received! Header:\n" + header);
				return;
			}
			std::string msgid(p.toString() +'/'+ wrapper.fileName());
			if (wrapper.isNewMsg()) {  // a new message is created
				MsgSet[msgid] = Message();
				wrapper.unwrap(MsgSet[msgid]);
			}
			if (MsgSet.find(msgid) == MsgSet.end()) {
				// first block of header information missing, drop data
				return;
			}
			// then read one block according to the header info
			size_t len = wrapper.rangeEnd() - wrapper.rangeStart()+1;
			if (len>0 && !wrapper.isACK()) {
				char * buff = new char[len];
				s.recvAll(buff, len);
				MsgSet[msgid].push(DataBlock(buff, len));
				delete buff;
			}
			processMsg(wrapper, msgid, p);
		}

		///////////////////////////////////////////////////
		// main part
		void run() {
			try {
				std::string header;
				// receiver will read one line
				while (header != "quit") {
					// first read one line from the socket, HTTP header
					header = s.readLine();
					if (header.empty() || header=="quit") {
						// wait 1.5 sec to continue, prevent 100% CPU
						::Sleep(1500);
						continue;
					}
					Peer p(s);
					//ch.log("<< Data block received from "+ p.toString() +". Header:\n  "+ header);
					readMsg(header, p);
				}
			}
			catch (std::exception& ex) {
				ch.log("Reading received data block error: "+ std::string(ex.what()));
			}
			catch (...) {
				ch.log("Reading received data block error");
			}
		}
	public:
		// constructor
		ClientHandlerThread(Socket _s, messageQ& _q, Channel& _ch) : s(_s), q(_q), ch(_ch) {}
	};

	///////////////////////////////////////////////////
	// listener thread
	// Hiding details from upper layer, thus I define it inside Channel
	class ListenThread : public threadBase {
		Channel& ch;
		messageQ& q;
		SocketListener* sl;

		///////////////////////////////////////////////////
		// run part
		void run() {
			try {
				while (1) {
					SOCKET s = sl->waitForConnect();
					ClientHandlerThread* pCht = new ClientHandlerThread(s, q, ch);
					pCht->start();
				}
			}
			catch (std::exception& ex) {
				ch.log("Listening error: "+ std::string(ex.what()));
			}
			catch (...) {
				ch.log("Listening error");
			}
		}
	public:
		///////////////////////////////////////////////////
		// constructor
		ListenThread(size_t port, Channel& _ch, messageQ& _q) : ch(_ch), q(_q), sl(new SocketListener(port)) { }
	};

	///////////////////////////////////////////////////
	// sender thread
	class SendThread : public threadBase
	{
		Channel& ch;
		Socket s;

		///////////////////////////////////////////////////
		// send message
		void sendMsg(MsgPair& msg) {
			if (msg.second.isBinary() && msg.second.size() <1) return;	// corrupted file message (file doesn't exist)
			HttpWrapper wrapper;
			size_t range = 0;
			wrapper.wrap(msg.second);
			wrapper.replyPort() = ch._listen_port;
			for (auto it = msg.second.begin(); it != msg.second.end(); it++) {
				// calculate current content range
				wrapper.rangeStart() = range;
				wrapper.rangeEnd() = (range += it->size()) -1;
				if (wrapper.rangeEnd() < wrapper.rangeStart()) wrapper.rangeEnd() = wrapper.rangeStart();	// happens when this is an ACK msg
				std::string& header = it->header() = wrapper.writeHeader();
				size_t len = header.length() + it->size();
				char * buff = new char[len];
				char * data = it->data();
				for (size_t i=0; i<header.length(); i++)
					buff[i] = header[i];
				for (size_t i = header.length(); i<len; i++)
					buff[i] = data[i-header.length()];
				if (!s.sendAll(buff, len)) {	// unable to send all data
					ch.log("Bad status in sending thread");
					return;
				}
				//ch.log(">> Data block sent to "+ msg.first.toString() +". Header: \n  "+ header);
				delete buff;
			}
		}

		///////////////////////////////////////////////////
		// main part
		void run() {
			try {
				while (1) {
					MsgPair msg = ch.sendQ.deQ();
					//ch.log("Sending Message..");
					if (!s.connect(msg.first.remote, msg.first.rport)) {	// connect to remote peer
						ch.log("Couldn't connect to "+ msg.first.remoteHost());
						continue;
					}
					else {	// connection failed to be established
						msg.first.fill(s);
						//ch.log("Connected to "+ msg.first.toString());
					}
					sendMsg(msg);
					s.writeLine("quit");
					s.disconnect();	// disconnect immediately after sending message every time
					//ch.log("Message sent! Disconnected with "+ msg.first.toString());
				}
			}
			catch (std::exception& ex) {
				ch.log("Sending data block error: "+ std::string(ex.what()));
			}
			catch (...) {
				ch.log("Sending received data block error");
			}
		}
	public:
		///////////////////////////////////////////////////
		// constructor
		SendThread(Channel& _ch) : ch(_ch) {}
	};

	SendThread* sth;	// send thread

public:
	///////////////////////////////////////////////////
	// constructor which will listen to random port
	Channel(const std::string& name) : 
		channelName(name), _enableACK(true), sth(new SendThread(*this)) {
			sth->start();
	}

	///////////////////////////////////////////////////
	// constructor
	Channel(const std::string& name, const Peer& _p) :
		channelName(name), _enableACK(true), _remotePeer(_p), sth(new SendThread(*this)) {
			// start send thread
			sth->start();
	}
	
	///////////////////////////////////////////////////
	// return remote peer
	Peer& remotePeer() {
		return _remotePeer;
	}

	///////////////////////////////////////////////////
	// enable ACK
	bool& enableACK() {
		return _enableACK;
	}

	///////////////////////////////////////////////////
	// connect to remote peer
	void send(const Peer& p, const Message& msg) {
		if (p.lport == 0) return;
		MsgPair msgP(p, msg);
		sendQ.enQ(msgP);
	}

	///////////////////////////////////////////////////
	// send to binded remote peer
	void send(const Message& msg) {
		if (_remotePeer.rport == 0 || _remotePeer.remote.empty()) return;
		send(_remotePeer, msg);
	}

	///////////////////////////////////////////////////
	// start listen thread, binding service to one specific port
	template <typename CallBackF>
	void listen(size_t port, CallBackF& f) {
		messageQ receiveQ;	// receive buffer queue
		std::ostringstream ss;
		ss << "Start listening on port "<< port;
		log(ss.str());
		try {
			ListenThread* lths = new ListenThread(_listen_port = port, *this, receiveQ);
			size_t count = 0;
			lths->start();
			while (1) {	// monitor the receive Q
				count++;
				std::ostringstream os;
				//os << "Message#" << count << " is received!";
				MsgPair msg = receiveQ.deQ();
				log(os.str());
				// now call back
				f(msg);
				::Sleep(200);	// sleep so other threads can join
			}
			delete lths;
		}
		catch(std::exception& ex) {
			log("Listen process error: "+ std::string(ex.what()));
		}
		catch(...)
		{
			log("Listen process error.");
		}
	}

	///////////////////////////////////////////////////
	// listen to default local port
	template <typename CallBackF>
	void listen(CallBackF& f) {
		if (_remotePeer.lport == 0) {
			// generate a random port number
			srand((unsigned)time(NULL));
			// sleep random time, then generate seed again
			_remotePeer.lport = 10000 + rand() % 20000; // random port 50000~59999
			log("Local port unassigned. Use random port.");
		}
		listen<CallBackF>(_remotePeer.lport, f);
	}

	///////////////////////////////////////////////////
	// print message to screen
	void log(const std::string& msg) {
		if (msg.length() < 1) return;
		sout << locker << " " << channelName <<" "<< msg;
		if (msg[msg.length()-1] != '\n')
			sout << "\n";
		sout << unlocker;
	}
};

#endif