
#include "LogFile.h"
#include <io.h>
#include <process.h>
#include <thread>
#include <atomic>

#include <concurrent_queue.h>
#include "IDefine.h"
#include "ShareFunction.h"
using namespace io;


Concurrency::concurrent_queue<S_LOG_BASE*>  __logs;//封装文件
Concurrency::concurrent_queue<S_LOG_BASE*>  __pools;//回收池



void io::getPoolCount(int& runnum, int& poolnum)
{
	poolnum = __pools.unsafe_size();
	runnum  = __logs.unsafe_size();
}
S_LOG_BASE* popPool()
{
	S_LOG_BASE* log = nullptr;
	if (__pools.empty() == true)
	{
		log = new S_LOG_BASE();
		log->reset();
	}
	else
	{
		__pools.try_pop(log);
		if (log == nullptr)
		{
			log = new S_LOG_BASE();
			log->reset();
		}

	}
	
	return log;
}


char temp_str[1024 * 100];
void io::pushLog(int type,const char * context, ...)
{
	auto log = popPool();
	log->reset();
	log->type = type;

	char ftime[30];
	share::formatTime(time(0),ftime);
	
	memset(temp_str, 0, 1024 * 100);

	va_list args;
	va_start(args, context);
	vsnprintf_s(temp_str, 1024*10, context, args);

	sprintf_s(log->text, "%s--%s", ftime, temp_str);

	__logs.push(log);

	va_end(args);
}


void writelog(S_LOG_BASE* log)
{
	SYSTEMTIME sys;
	std::string filenamex = "log/";
	GetLocalTime(&sys);

	
	char fpath[100];
	sprintf_s(fpath, "%d-%d-%d.txt", sys.wYear, sys.wMonth, sys.wDay);

	switch (log->type)
	{
	case EFT_RUN:
		filenamex = "log/run";
	    break;
	case EFT_ERR:
		filenamex = "log/err";
		break;
	defalut:
		filenamex = "log/other";
		break;
	}

	log->file.fOpen(filenamex.c_str(), fpath);
	if (log->file.m_File == nullptr) return;
	log->file.fWrite(log->text);
	log->file.fClose();
}

int run()
{
	printf("run logthread...\n");

	while (true)
	{
		while (!__logs.empty())
		{
			S_LOG_BASE* log = nullptr;
			__logs.try_pop(log);
			if (log == nullptr) break;

			writelog(log);

			__pools.push(log);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	printf("exit logThread...\n");
	return -1;
}

bool io::runLogThread()
{	
	for (int i = 0; i < MAX_LOG_LENGTH; i++)
	{
		S_LOG_BASE* log = new S_LOG_BASE();
		log->reset();
		__pools.push(log);
	}


	std::thread  th(&run);
	th.detach();
	return false;
}

//******************************************************************************

LogFile::LogFile() 
{
}

void LogFile::fOpen(const char* filepath, const char* filename)
{
	//1、判断第一层目录是不是存在？ 不存在创建顶级目录
	std::string ss = "log";
	std::string str = func::FileExePath + ss;
	int isexist1 = _access(str.c_str(), 0);
	if (isexist1 == -1)
	{
		CreateDirectoryA(str.c_str(), NULL);
	}

	//文件名
	char fname[MAX_FILENAME_LEN];
	memset(fname, 0, MAX_FILENAME_LEN);
	sprintf_s(fname, "%s%s\\%s", func::FileExePath, filepath, filename);

	//bool iswrite = true;//是否可以写？
	int32_t isexist = _access(fname, 0);
	if (isexist == -1)
	{
		//创建文件夹
		char fpath_txt[MAX_FILENAME_LEN];
		memset(fpath_txt, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath_txt, "%s%s", func::FileExePath, filepath);
		CreateDirectoryA(fpath_txt, NULL);
		//创建文件
		m_File = _fsopen(fname, "wt+", _SH_DENYNO);
		if (m_File) fclose(m_File);
	}
	m_File = _fsopen(fname, "at+", _SH_DENYNO);
}

LogFile::~LogFile()
{
	this->fClose();
}
void LogFile::fClose()
{
	if (m_File)
	{
		fclose(m_File);
	}
}
void LogFile::fWrite(const char* text)
{
	if (!m_File) return;
	fputs(text, m_File);
	fflush(m_File);
}





