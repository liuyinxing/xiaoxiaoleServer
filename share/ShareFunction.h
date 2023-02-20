#ifndef  ____SHAREFUNCTION_H
#define  ____SHAREFUNCTION_H 

#define CMD_REIGSTER      100
#define CMD_UPDATELOGIN   200
#define CMD_CREATEROLE    300

#define CMD_LOGIN      1000
#define CMD_MOVE       2000
#define CMD_PLAYERDATA 3000
#define CMD_LEAVE      4000

#define USER_MAX_MEMBER 20
#define MAX_LINE_COUNT  10
#define MAX_MAP_ID      110

#include "IDefine.h"
#include <string>
#include <time.h> 
#include <cstring>
#include <string.h>

namespace share
{
	//enum E_MEMBER_STATE
	//{
	//	M_FREE = 0x00,	//使用-空闲
	//	M_REQUESTING,   //正在请求数据中
	//	M_LOGIN,		//使用-登录中
	//	M_SAVING,       //保存中
	//	M_LIMIT 		//禁用		
	//};

	// 字符串分割函数
	inline std::vector<std::string> split(std::string str, std::string pattern, bool isadd = false)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		if (isadd) str += pattern;//扩展字符串以方便操作

		int size = str.size();
		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);//pos为起始位置（默认为0），n为结束位置（默认为npos）
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}

	inline void serverTypeToString(int sid, std::string& str)
	{
		if (sid >= 1000 && sid < 2000) str = "DB";
		else if (sid == 2000) str = "Center";
		else if (sid >= 3000 && sid < 4000)  str = "Game";
		else if (sid >= 4000 && sid < 5000) str = "Gate";
		else if (sid >= 5000 && sid < 6000)  str = "Login";
		else if (sid == 6000) str = "Chat";
		else if (sid == 7000) str = "Team";
		else if (sid == 8000) str = "Log";
		else str = "User";
		
	}
	inline int serverIDToType(int sid)
	{
		if (sid >= 1000 && sid < 2000) return func::S_TYPE_DB;
		else if (sid == 2000) return func::S_TYPE_CENTER;
		else if (sid >= 3000 && sid < 4000) return func::S_TYPE_GAME;
		else if (sid >= 4000 && sid < 5000) return func::S_TYPE_GATE;
		else if (sid >= 5000 && sid < 6000) return func::S_TYPE_LOGIN;
		else if (sid >= 7000 && sid < 8000) return func::S_TYPE_REDIS;
		else return func::S_TYPE_USER;
	}

	//生成一个md5码
	inline std::string createMD5()
	{
		//生成一个key秘钥 用于验证
		srand(time(NULL));
		u32 rcode = rand() % 1000 + 100;
		char key[30];
		char md5[50];
		sprintf(key, "%s_%d", func::__ServerInfo->SafeCode, rcode);
		memset(md5, 0, 50);
		if (func::MD5str != NULL) func::MD5str(md5, (unsigned char*)key, strlen(key));

		md5[49] = '\0';
		std::string str(md5);
		return str;
	}

	inline void formatTime(time_t time1, char* szTime)
	{
		struct tm tm1;
#ifdef WIN32  
		tm1 = *localtime(&time1);
#else  
		localtime_r(&time1, &tm1);
#endif  
		sprintf(szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
			tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday,
			tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	}

	
	

	extern u8   __ServerLine;//服务器线路 gameserver 使用
	extern u8   __MapUsingList[MAX_MAP_ID];//GameServers使用
	extern u32  __MapidToGameServerids[MAX_LINE_COUNT][MAX_MAP_ID];//通过地图ID找到对应的游戏服务器ID Gate Chat

	extern char* Utf8ToUnicode(char* szU8);
	extern void UnicodeToUtf8(const wchar_t* unicode, char* src);


	extern bool InitData(bool isServer);
	extern bool is_str_utf8(const char* str);
	extern bool is_str_gbk(const char* str);
	extern bool Is_Utf8_String(const char* utf, int length);

	extern void  updateMemory(u32& curvalue, u32& maxvalue);
	extern u32 findGameServerID(const u8 line, const u32 mapid);

}


#endif