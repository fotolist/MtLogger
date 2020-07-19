// MtLoggerDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "MtLogger.h"

using namespace NS_MtLogger;

void LogTestThread()
{
	//MtLogger MtLoggerTest;	//No need to define another MtLogger here
	for (int i = 0; i < 100; i++)
	{
		MtLogger::WriteLogFile(L"Thread 2 msg " + std::to_wstring(i + 1), L"Test", MsgLvl::I);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	MtLogger mtLogger;			//Only 1 MtLogger should be defined at most beginning.
	//MtLogger mtLogger1;		//if one more MtLogger is defined anywhere, a runtime_error exception("MtLogger duplicate definition") will be thrown.

	//Creat another test thread.
	std::thread t1(LogTestThread);
	t1.detach();

	//Main test
	for (int i = 0; i < 100; i++)
	{
		MtLogger::WriteLogFile(L"Thread 1 msg " + std::to_wstring(i + 1), L"Test", MsgLvl::I);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}
