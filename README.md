# MtLogger
A simple C++17 logger design which can be used in Windows and Linux.

## Basic Usage:

void LogTestThread()
{
	for (int i = 0; i < 100; i++)
	{
		MtLogger::WriteLogFile(L"Thread 2 msg " + std::to_wstring(i + 1), L"Test", MsgLvl::I);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	MtLogger mtLoggerAnyName;
  
	std::thread t1(LogTestThread);
	t1.detach();

	for (int i = 0; i < 100; i++)
	{
		MtLogger::WriteLogFile(L"Thread 1 msg " + std::to_wstring(i + 1), L"Test", MsgLvl::I);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}
