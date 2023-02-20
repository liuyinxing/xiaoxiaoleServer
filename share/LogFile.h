#ifndef ___LogFile_H
#define ___LogFile_H
#include <stdint.h>
#include <string>

#define MAX_TEXT_LENGTH 1024
#define MAX_LOG_LENGTH  1000

namespace io
{
	enum E_FILE_TYPE
	{
		EFT_RUN = 0,
		EFT_ERR = 1
	};

	class LogFile
	{
	public:
		FILE*	 m_File;
	public:
		LogFile();
		~LogFile();
		void fOpen(const char* filepath, const char* filename);
		void fClose();
		void fWrite(const char* text);
	};

	struct S_LOG_BASE
	{
		int  type;
		char text[MAX_TEXT_LENGTH];
		LogFile file;
		inline  void reset()
		{
			type = 0;
			file.m_File = nullptr;
			memset(text, 0, MAX_TEXT_LENGTH);
		}
	};



	bool runLogThread();
	extern void getPoolCount(int& runnum, int& poolnum);
	extern void pushLog(int type,const char* context, ...);
}
#endif