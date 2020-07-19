#pragma once

#if defined(__linux__) || defined(__linux)
#define MT_OS_LINUX
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdexcept>

#ifdef MT_OS_LINUX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>	
#else
#include <Windows.h>
#endif

namespace NS_MtLogger
{
	class tm_ex
	{
	public:
		struct tm _tm { 0 };
		int _tm_ms{ 0 };	// milliseconds after the second - [0, 999], extended

		void from_tm(const struct tm& __tm) {
			_tm_ms = 0;
			_tm = __tm;
		}

		struct tm to_tm() {
			return _tm;
		}
	};

	class MtTime
	{
	public:
		static tm_ex GetLocalTimeEx();				//Get current local time
		static std::wstring GetDateStr();			//20100705
		static std::wstring GetTimeStr();			//2010-07-05T21:05:32.072 -> [dateX]["T"][timeX]		
	};

	enum class MsgLvl
	{
		I = 0, W = 1, E = 2
	};

	class LogMsg
	{
	public:
		friend class MtLogger;
	private:		
		std::wstring msg{ L"" };	//Log information		
		std::wstring file{ L"" };	//Log file name -> ./logs/file		
		std::wstring level{ L"I" };	//Log information severity ->E, W, I

		LogMsg() = default;
		LogMsg(std::wstring _msg, std::wstring _file, std::wstring _level) : msg(_msg), file(_file), level(_level) {}
	};

	class MtLogger
	{
	public:
		MtLogger();
		~MtLogger();		
		static void WriteLogFile(const std::wstring& msg, const std::wstring& file, MsgLvl msgLvl = MsgLvl::I);
	private:
		static void Init();
		static void Unload();
		static void WriteLogFile(const std::wstring& msg, const std::wstring& file, const std::wstring& msgLvl);
		static void LogMsgCollect(const LogMsg& logMsg);
		static bool LogMsgOutput(const LogMsg& logMsg);
		static void LogMsgSpooler();

		static std::wstring StringToWstring(std::string const& str);
		static std::string WstringToString(std::wstring const& wstr);	//Missing information possibly 

		static std::list<LogMsg> logMsgList;
		static std::mutex logMutex;
		static std::condition_variable logCond;

		static bool toExit;
		static bool initialized;
		static bool logThreadExited;
	};
}

