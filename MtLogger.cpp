#include "MtLogger.h"

namespace NS_MtLogger
{
	std::list<LogMsg> MtLogger::logMsgList;
	std::mutex MtLogger::logMutex;
	std::condition_variable MtLogger::logCond;
	bool MtLogger::logThreadExited = false;
	bool MtLogger::initialized = false;
	bool MtLogger::toExit = false;

	tm_ex MtTime::GetLocalTimeEx()
	{	
		tm_ex tmEx{};
		auto tp_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		time_t t = std::chrono::system_clock::to_time_t(tp_ms);
		struct tm tmNow;
#ifdef MT_OS_LINUX
		localtime_r(&t, &tmNow);
#else
		localtime_s(&tmNow, &t);
#endif
		tmEx.from_tm(tmNow);
		tmEx._tm_ms = tp_ms.time_since_epoch().count() % 1000;
		return tmEx;
	}

	std::wstring MtTime::GetDateStr()		//20100705 -> [date]
	{
		std::wstringstream timestr;
		tm_ex tmEx = GetLocalTimeEx();
		timestr << std::put_time(&tmEx._tm, L"%Y%m%d");
		return timestr.str();
	}

	std::wstring MtTime::GetTimeStr()		//2010-07-05T21:05:32.072 -> [dateX]["T"][timeX]
	{
		std::wstringstream timestr;
		tm_ex tmEx = GetLocalTimeEx();
		timestr << std::put_time(&tmEx._tm, L"%Y-%m-%d") << L"T" << std::put_time(&tmEx._tm, L"%H:%M:%S") << L"." << std::setfill(L'0') << std::setw(3) << tmEx._tm_ms;
		return timestr.str();
	}

	MtLogger::MtLogger()
	{
		Init();
	}

	MtLogger::~MtLogger()
	{
		Unload();
	}

	void MtLogger::Init()
	{
		if (!initialized) {
			toExit = false;
			std::thread t1(LogMsgSpooler);
			t1.detach();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			initialized = true;
		}
		else
		{
			throw std::runtime_error("MtLogger duplicate definition");
		}
	}

	void MtLogger::Unload()
	{		
		if (initialized) {
			toExit = true;
			auto tp_start = std::chrono::system_clock::now();			
			while (!logThreadExited) {
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				auto tp_now = std::chrono::system_clock::now();
				if (std::chrono::duration_cast<std::chrono::seconds>(tp_now - tp_start).count() > 30) {	//In some case, if there is to much message cached(e.g. >10000), exception thrown.
					break;
				}
			}
			initialized = false;
		}
	}

	std::wstring MtLogger::StringToWstring(std::string const& str)
	{
		std::wstring wstr(str.length(), L' ');
		std::copy(str.begin(), str.end(), wstr.begin());
		return wstr;
	}

	std::string MtLogger::WstringToString(std::wstring const& wstr)
	{
		std::string str(wstr.length(), ' ');
		std::copy(wstr.begin(), wstr.end(), str.begin());
		return str;
	}

	void MtLogger::WriteLogFile(const std::wstring& msg, const std::wstring& file, MsgLvl msgLvl)
	{
		std::wstring level{};
		switch (msgLvl)
		{
		case MsgLvl::I:
			level = L"I";
			break;
		case MsgLvl::W:
			level = L"W";
			break;
		case MsgLvl::E:
			level = L"E";
			break;
		}
		WriteLogFile(msg, file, level);
	}

	void MtLogger::WriteLogFile(const std::wstring& msg, const std::wstring& file, const std::wstring& msgLvl)
	{
		LogMsg logMsg{ msg, file, msgLvl };
		LogMsgCollect(logMsg);
	}

	void MtLogger::LogMsgCollect(const LogMsg& logMsg)
	{
		std::unique_lock<std::mutex> locker(logMutex);
		logMsgList.push_front(logMsg);
		locker.unlock();
		logCond.notify_one();
	}

	void MtLogger::LogMsgSpooler()
	{
		while (!toExit || !logMsgList.empty()) {
			std::unique_lock<std::mutex> locker(logMutex);	//After this, logMuext is locked
			while (logMsgList.empty()) {
				logCond.wait_for(locker, std::chrono::microseconds(100));	//In waiting state, logMutex is unlocked, after it's been awaken, logMutex is locked
				if (toExit) {
					break;
				}
			}
			if (logMsgList.empty()) {	//Notified by exit or other uncertain event
				locker.unlock();
				continue;
			}
			else {	//Notified by log filling in event
				LogMsg logMsg = logMsgList.back();
				logMsgList.pop_back();
				locker.unlock();
				LogMsgOutput(logMsg);
			}
		}
		logThreadExited = true;
	}

	bool MtLogger::LogMsgOutput(const LogMsg& logMsg)
	{
		std::wstringstream ssLogFileName{};
		std::wstring path = L"./logs/";
		
#ifdef MT_OS_LINUX
		std::string str_path = WstringToString(path);
		if (access(str_path.c_str(), 0) != 0) {
			mkdir(str_path.c_str(), 666);
		}
#else
		if (_waccess(path.c_str(), 0) != 0) {
			_wmkdir(path.c_str());
		}
#endif
		ssLogFileName << path << MtTime::GetDateStr() << L"_" << logMsg.file << L".log";	//20200327_log_Test.log	
		std::string strLogFileName = WstringToString(ssLogFileName.str());
		std::wofstream oLogFile(strLogFileName.c_str(), std::ios::out | std::ios::app);
		if (!oLogFile) {
			return false;
		}
		oLogFile << L"[" << MtTime::GetTimeStr() << L"] [" << logMsg.level << L"] " << logMsg.msg << std::endl;
		oLogFile.close();
		return true;
	}	
}