#ifndef SOCKETS_H
#define SOCKETS_H
/////////////////////////////////////////////////////////////////////
// Sockets.h   -  Provides basic network communication services    //
// ver 3.1                                                         //
// Language:      Visual C++, 2005                                 //
// Platform:      Dell Dimension 9150, Windows XP Pro, SP 2.0      //
// Application:   Utility for CSE687 and CSE775 projects           //
// Author:        Jim Fawcett, Syracuse University, CST 2-187      //
//                (315) 443-3948, jfawcett@twcny.rr.com            //
/////////////////////////////////////////////////////////////////////
/*
   Module Operations:
   ==================
   This module provides network communication services, using 
   WinSock2, a nearly Berkley Sockets compliant implementation.
   Three classes are provided:

   SocketSystem:
   -------------
   provides WinSock loading, unloading and a few program wide services.
   A recent change has ensured that the WinSock library is only loaded
   once, no matter how many times you construct SocketSystem objects.
   So now, the Socket class has a SocketSystem instance so you don't
   have to do an explicit creation of a SocketSystem object before
   creating a Socket.

   Socket:
   -------
   Provides connect request and read/write services.
   
   SocketListener:
   ---------------
   Provides connection handling.
   
   Public Interface:
   =================
   SocketListener listener(2048);             // create listener
   Socket recvr = listener.waitForConnect();  // start listener listening
   Socket sendr;                              // create sending socket
   sender.connect("\\localhost",2048);        // request a connection
   const char* msg = "this is a message"; 
   sender.sendAll(msg,strlen(msg)+1);         // send msg and terminating null
   sender.sendAll("quit",strlen("quit")+1);   // send another msg

   char* buffer[1024];                        // receive buffer
   recvr.recvAll(buffer,strlen(msg)+1);       // copy data when available
   std::cout << "\n  recvd: " << buffer;
   recvr.recvA;;(buffer,strlen("quit")+1);    // get more data
   std::cout << "\n  recvd: " << buffer;

   sender.WriteLine("this is a line");        // will append newline          
   std::string reply = recvr.ReadLine();      // removes newline

   recvr.disconnect();                        // graceful shutdown
   sender.disconnect();                       // graceful shutdown
*/
//
/*
   Build Process:
   ==============
   Required Files:
     Sockets.h, Sockets.cpp

   Compile Command:
   ================
   cl /EHsc /DTEST_SOCKETS Sockets.cpp wsock32.lib user32.lib

   Maintenance History:
   ====================
   ver 3.1 : 29 Mar 2013
   - changed ReadLine to readLine   -- breaking change
   - changed WriteLine to writeLine -- breaking change
   - removed a check for bytes > 0 in ReadLine()
   - modified error handling in sendAll
   ver 3.0 : 15 Apr 2012
   - changed send to sendAll -- breaking change
   - changed recv to recvAll -- breaking change
   - added ReadLine and WriteLine
   - added bytesLeft to avoid blocking call
   - now allow compiler generated copy and assignment
   ver 2.1 : 17 Apr 2010
   - added planned changes, below
   ver 2.0 : 13 Apr 2010
   - made copy constructor and standard assignment operator private.
   - these operations can be made to work, but you need to duplicate
     the SOCKET handle to make the SOCKET object reference counting
     work as expected.  Not implemented yet.
   ver 1.9 : 05 May 2009
   - fixed bug, found by Phil Pratt-Szeliga, in 
     SocketListener::WaitForConnection() due to allowing a
     client handling thread to be created with an invalid socket.
     That was fixed in SocketListener::WaitForConnect().
   ver 1.8 : 04 Apr 08
   - discovered that WinSock connect function does not always return 
     appropriate error message on failure to connect (this is a new
     bug in WinSock, I believe) so added a remote port check, which
     returns -1 on failure to connect.
   ver 1.7 : 27 Mar 08
   - added throwing exception in socket.connect(...)
   ver 1.6 : -1 May 07
   - Estepan Meliksetian reported a problem with disconnect preventing a reconnect.
     This has been fixed by setting the socket s_ to INVALID_SOCKET in the disconnect
     function, and testing for INVALID_SOCKET in the connect function.
   ver 1.5 : 05 Apr 07
   - removed redundant Winsock socket initialization from Socket copy ctor.
     Bug reported by Vishal Chowdhary.
   ver 1.4 : 01 Apr 07
   - removed some commented code
   ver 1.3 : 27 Mar 07
   - added SocketSystem members to Socket and SocketListener
   ver 1.2 : 18 Feb 07
   - cosmetic changes
   ver 1.0 : 11 Feb 07
   - first release, a major rewrite of an earlier socket demo

   Planned Changes:
   ================
   - Add reference counting by duplicating socket handle and change access
     of copy and assignment to public.

*/

#include <string>
#include <winsock2.h>

/////////////////////////////////////////////////////////////////////
// SocketSystem class loads and unloads WinSock library
// and provides a few system services

class Socket;

class SocketSystem
{
public:
  SocketSystem();
  ~SocketSystem();
  std::string getHostName();
  std::string getNameFromIp(const std::string& ip);
  std::string getIpFromName(const std::string& name);
  std::string getRemoteIP(Socket* pSock);
  int getRemotePort(Socket* pSock);
  std::string getLocalIP();
  int getLocalPort(Socket* pSock);
  std::string GetLastMsg(bool WantSocketMsg=true);
private:
  static long count;
};

/////////////////////////////////////////////////////////////////////
// Socket class provides basic connect, read, and write operations

class Socket
{
public:
  Socket();
  Socket(const Socket& sock);
  Socket(SOCKET s);
  ~Socket();
  Socket& operator=(const Socket& sock);
  Socket& operator=(SOCKET sock);
  operator SOCKET ();
  bool connect(std::string url, int port, bool throwError=false, size_t MaxTries=50);
  void disconnect();
  bool error() { return (s_ == SOCKET_ERROR); }
  int send(const char* block, size_t len);
  int recv(char* block, size_t len);
  int bytesLeft();
  bool sendAll(const char* block, size_t len, bool throwError=false);
  bool recvAll(char* block, size_t len, bool throwError=false);
  bool writeLine(const std::string& str);
  std::string readLine();
  HANDLE getHandle() { return (HANDLE)s_; }
  SocketSystem& System() { return ss_; }
private:
  SOCKET s_;
  SocketSystem ss_;
};

/////////////////////////////////////////////////////////////////////
// SocketListener class waits for connections, then delivers
// connected socket

class SocketListener
{
public:
  SocketListener(int port);
  ~SocketListener();
  SOCKET waitForConnect();
  void stop();
  long getInvalidSocketCount();
private:
  SOCKADDR_IN tcpAddr;
  Socket s_;
  SocketSystem ss_;
  volatile long InvalidSocketCount;
};

inline long SocketListener::getInvalidSocketCount() 
{ 
  return InvalidSocketCount; 
}

#endif
