
//////////////////////////////////////////////////////////////////////////                
//  MockChannel.cpp - A bridge that connects C# GUI and C++ background  //
//	Language:		Visual C++ 2011      								//
//	Platform:		Dell Studio 1558, Windows 7 Pro x64 Sp1				//
//	Application:	CIS 687 / Project 4, Sp13                       	//
//	Author:			Kevin Wang, Syracuse University						//
//					kevixw@gmail.com									//
//////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include "MockChannel.h"
#include "MockMessenger.h"
#include "../BlockingQueue/BlockingQueue.h"
#include "../Threads/Threads.h"
#include "../Threads/locks.h"

using namespace mockChannel;

/////////////////////////////////////////////////////////////////////////
// Helper message converters

typedef std::string stdMessage;

stdMessage ConvertMsgDown(StrMessage^ msg)
{
	stdMessage smsg;
	for(int i=0; i<msg->Length; ++i)
		smsg += (int)msg[i];
	return smsg;
}

StrMessage^ ConvertMsgUp(const stdMessage& smsg)
{
	StrMessage^ msg;
	for(size_t i=0; i<smsg.size(); ++i)
		msg += (wchar_t)smsg[i];
	return msg;
}

typedef BlockingQueue<std::string> BQueue;

/////////////////////////////////////////////////////////////////////////
// Fawcett thread, used to make sure that will work with C++\CLI code

class Thread : public tthreadBase
{
	BQueue* pInQ_;
	BQueue* pOutQ_;
	static MockMessenger proc;
public:
	Thread(BQueue* pInQ, BQueue* pOutQ) : pInQ_(pInQ), pOutQ_(pOutQ) {}
	void run()
	{
		while(true)
		{
			stdMessage msg = pInQ_->deQ();
			if(msg == "ChannelShutDown")
				break;
			proc(msg);	// process message
		}
	}
};

MockMessenger Thread::proc;

/////////////////////////////////////////////////////////////////////////
// ref class MockChannel echos messages from input queue to output queue

ref class MockChannel : public IChannel
{
public:
	MockChannel() : pInQ(&Status::inQ()), pOutQ(&Status::outQ()) 
	{
		Thread* pThread = new Thread(pInQ, pOutQ);
		pThread->start();
	}
	~MockChannel()
	{
		delete pInQ;
		delete pOutQ;
	}

	virtual void shutDown() override
	{
		postMessage("ChannelShutDown");
	}

	virtual StrMessage^ getMessage() override
	{
		stdMessage smsg = pOutQ->deQ();
		return ConvertMsgUp(smsg);
	}
	virtual void postMessage(StrMessage^ msg) override
	{
		stdMessage smsg = ConvertMsgDown(msg);
		pInQ->enQ(smsg);
	}

	void showQueues(const std::string& msg)
	{
		sout << locker << "\n  " << msg << unlocker;
		sout << locker << "\n inQ has " << pInQ->size() << " messages";
		sout << " and outQ has " << pOutQ->size() << " messages" << unlocker;
		sout.flush();
	}

private:
	BQueue* pInQ;
	BQueue* pOutQ;
};

IChannel^ IChannel::CreateChannel()
{
	return gcnew MockChannel();
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >--------------------------------------------
int main()
{
	StrMessage^ testMessage = "testMessage";
	stdMessage sTestMessage = ConvertMsgDown(testMessage);
	std::cout << "\n  " << sTestMessage;
	StrMessage^ resultMessage = ConvertMsgUp(sTestMessage);
	Console::Write("\n  {0}\n", resultMessage);

	IChannel^ chan = IChannel::CreateChannel();
	StrMessage^ msg = "a message";
	chan->postMessage(msg);
	StrMessage^ retMsg = chan->getMessage();
	Console::Write("\n  {0}\n\n", retMsg);
	chan->shutDown();
}

#endif