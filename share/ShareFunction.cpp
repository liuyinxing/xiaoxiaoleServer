#include "ShareFunction.h"
#include "IDefine.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include "tinyxml/md5.h"
#include "INetBase.h"
#include <process.h>

#include <Windows.h> 
#include <psapi.h>
#pragma comment(lib,"psapi.lib")


using namespace func;
namespace share
{
	u8   __ServerLine = 0;//服务器线路
	u8   __MapUsingList[MAX_MAP_ID];//地图使用列表
	u32  __MapidToGameServerids[MAX_LINE_COUNT][MAX_MAP_ID];


	int LoadServerXML(const char* filename)
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf(fpath, "%s%s", func::FileExePath, filename);

		if (func::__ServerInfo == nullptr)
		{
			func::__ServerInfo = new func::ConfigXML();
			memset(func::__ServerInfo, 0, sizeof(func::ConfigXML));
		}

		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("load config_server.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("server");
		memcpy(__ServerInfo->SafeCode, xmlNode->Attribute("SafeCode"), 20);
		memcpy(__ServerInfo->Head, xmlNode->Attribute("Head"), 2);
		__ServerInfo->Port = atoi(xmlNode->Attribute("Port"));
		__ServerInfo->ID = atoi(xmlNode->Attribute("ID"));
		__ServerInfo->Type = func::GetServerType(__ServerInfo->ID);
		__ServerInfo->MaxUser = atoi(xmlNode->Attribute("MaxUser"));
		__ServerInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
		__ServerInfo->MaxAccpet = atoi(xmlNode->Attribute("MaxAccpet"));
		__ServerInfo->MaxRece = atoi(xmlNode->Attribute("MaxRece"));
		__ServerInfo->MaxSend = atoi(xmlNode->Attribute("MaxSend"));
		__ServerInfo->RCode = atoi(xmlNode->Attribute("CCode"));
		__ServerInfo->Version = atoi(xmlNode->Attribute("Version"));
		__ServerInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) * 1024;
		__ServerInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) * 1024;
		__ServerInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) * 1024;
		__ServerInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) * 1024;
		__ServerInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));
		__ServerInfo->TimerOne= atoi(xmlNode->Attribute("TimerOne"))*1024;
		
		return 0;

	}

	int LoadClientXML(const char* filename)
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf(fpath, "%s/%s", FileExePath, filename);

		if (__ClientInfo == nullptr)
		{
			__ClientInfo = new ConfigXML();
			memset(__ClientInfo, 0, sizeof(ConfigXML));
		}

		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("load config_client.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("client");
		memcpy(__ClientInfo->SafeCode, xmlNode->Attribute("SafeCode"), 20);
		memcpy(__ClientInfo->Head, xmlNode->Attribute("Head"), 2);
		__ClientInfo->MaxUser = atoi(xmlNode->Attribute("MaxUser"));
		__ClientInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
		__ClientInfo->RCode = atoi(xmlNode->Attribute("CCode"));
		__ClientInfo->Version = atoi(xmlNode->Attribute("Version"));
		__ClientInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) * 1024;
		__ClientInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) * 1024;
		__ClientInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) * 1024;
		__ClientInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) * 1024;
		__ClientInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));
		__ClientInfo->AutoTime = atoi(xmlNode->Attribute("AutoTime"));

		//获取子节点信息1  用于连接各个服务器
		xmlNode = xmlRoot->FirstChildElement("server");
		int num = atoi(xmlNode->Attribute("num"));
		char str[10];
		for (int i = 1; i <= num; i++)
		{
			memset(&str, 0, 10);
			sprintf(str, "client%d", i);
			xmlNode = xmlRoot->FirstChildElement(str);
			if (xmlNode == NULL) continue;

			ServerListXML* serverlist = new ServerListXML();
			memcpy(serverlist->IP, xmlNode->Attribute("IP"), 16);
			serverlist->Port = atoi(xmlNode->Attribute("Port"));
			serverlist->ID = atoi(xmlNode->Attribute("ID"));

			__ServerListInfo.push_back(serverlist);
		}
		
		//u32 type = serverIDToType(__ServerInfo->ID);
		//if (type != func::S_TYPE_TEAM &&
		//	type != func::S_TYPE_GAME &&
		//	type != func::S_TYPE_GATE) return 0;


		////获取子节点信息1  用于通过地图ID查找对于的游戏服务器ID
		//xmlNode = xmlRoot->FirstChildElement("gameserverid");
		//if (xmlNode != NULL)
		//{
		//	int num = atoi(xmlNode->Attribute("num"));
		//	char str[10];
		//	for (int i = 1; i <= num; i++)
		//	{
		//		memset(&str, 0, 10);
		//		sprintf(str, "gsid%d", i);
		//		xmlNode = xmlRoot->FirstChildElement(str);
		//		if (xmlNode == NULL) continue;

		//		std::string ids = xmlNode->Attribute("id");
		//		u32 serverid = atoi(xmlNode->Attribute("serverid"));
		//		u32  line = atoi(xmlNode->Attribute("line"));
		//		if (line >= MAX_LINE_COUNT) continue;

		//		if (serverid == __ServerInfo->ID)
		//		{
		//			__ServerLine = line;
		//			LOG_MSG("serverID:%d line:%d \n", serverid, line);
		//		}

		//		std::vector<std::string> arr = split(ids, ",", true);
		//		u32 size = arr.size();
		//		for (int i = 0; i < size; i++)
		//		{
		//			u32 mapid = atoi(arr[i].c_str());
		//			if (mapid >= MAX_MAP_ID) continue;
		//			__MapidToGameServerids[line][mapid] = serverid;
		//			if (serverid == __ServerInfo->ID)
		//			{
		//				__MapUsingList[mapid] = 1;
		//			}
		//			
		//		}
		//	}
		//}
		return 0;

	}

	int LoadGameServerXML(const char* filename)
	{
		std::string newpath = "";
		std::vector<std::string> arr = split(FileExePath, "\\", true);
		int size = arr.size() - 3;
		if (size < 0) return -1;

		for (u8 i = 0; i < size; i++)
		{
			newpath += arr[i] + "\\";
		}
		newpath += "share\\";

		printf("LoadGameServerXML :%s \n", newpath.c_str());



		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf(fpath, "%s/%s", newpath.c_str(), filename);

		if (__ClientInfo == nullptr)
		{
			__ClientInfo = new ConfigXML();
			memset(__ClientInfo, 0, sizeof(ConfigXML));
		}

		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("LoadGameServerXML.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  用于通过地图ID查找对于的游戏服务器ID
		TiXmlElement*  xmlNode = xmlRoot->FirstChildElement("gameserverid");
		if (xmlNode == NULL) return -1;
		
		int num = atoi(xmlNode->Attribute("num"));
		char str[10];
		for (int i = 1; i <= num; i++)
		{
			memset(&str, 0, 10);
			sprintf(str, "gsid%d", i);
			xmlNode = xmlRoot->FirstChildElement(str);
			if (xmlNode == NULL) continue;

			std::string ids = xmlNode->Attribute("id");
			u32 serverid = atoi(xmlNode->Attribute("serverid"));
			u32  line = atoi(xmlNode->Attribute("line"));
			if (line >= MAX_LINE_COUNT) continue;

			if (serverid == __ServerInfo->ID)
			{
				__ServerLine = line;
				LOG_MSG("serverID:%d line:%d \n", serverid, line);
			}

			std::vector<std::string> arr = split(ids, ",", true);
			u32 size = arr.size();
			for (int i = 0; i < size; i++)
			{
				u32 mapid = atoi(arr[i].c_str());
				if (mapid >= MAX_MAP_ID) continue;
				__MapidToGameServerids[line][mapid] = serverid;
				if (serverid == __ServerInfo->ID)
				{
					__MapUsingList[mapid] = 1;
				}

			}
		}
		//获取子节点信息1  主要用于搜集所有服务器的内存 在线人数情况
		xmlNode = xmlRoot->FirstChildElement("serverTotal");
		if (xmlNode != NULL)
		{
			int num = atoi(xmlNode->Attribute("num"));
			char str[10];
			for (int i = 1; i <= num; i++)
			{
				memset(&str, 0, 10);
				sprintf(str, "server%d", i);
				xmlNode = xmlRoot->FirstChildElement(str);
				if (xmlNode == NULL) continue;
				ServerListXML* serverlist = new ServerListXML();
				memcpy(serverlist->IP, xmlNode->Attribute("IP"), 16);
				serverlist->Port = atoi(xmlNode->Attribute("Port"));
				serverlist->ID = atoi(xmlNode->Attribute("ID"));
				__ServerList.push_back(serverlist);
			}
		}
		
		return 0;

	}



	bool InitData(bool isServer)
	{
		memset(__MapUsingList, 0, MAX_MAP_ID);
		for (int i = 0; i < MAX_LINE_COUNT; i++)
			memset(__MapidToGameServerids[i], 0, 4*MAX_MAP_ID);
		//设置函数指针
		func::MD5str = &utils::EncryptMD5str;
		//初始化数据
		func::InitData();



		//2、初始化XML
		if (isServer) {
			int errs = LoadServerXML("config_server.xml");
			if (errs < 0) return false;
		}
		else {
			int errs = LoadClientXML("config_client.xml");
			if (errs < 0) return false;
		}


	//	errs = LoadGameServerXML("config_gameserver.xml");
		


		//LOG_MSG("serverxml:%s-%d-%d\n", __ServerInfo->Head, __ServerInfo->ID, __ServerInfo->Port);
		//LOG_MSG("clientxml:%s-%d-%d\n", __ClientInfo->Head, __ClientInfo->Version, __ClientInfo->RCode);

		//for (int i = 0; i < __ServerListInfo.size(); i++)
		//	LOG_MSG("clientxml:%s-%d-%d\n", __ServerListInfo[i]->IP, __ServerListInfo[i]->Port, __ServerListInfo[i]->ID);
		return true;
	}


	char* Utf8ToUnicode(char* szU8)
	{
		//UTF8 to Unicode
		//预转换，得到所需空间的大小
		int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);
		//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
		wchar_t* wszString = new wchar_t[wcsLen + 1];
		//转换
		::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);
		//最后加上'\0'
		wszString[wcsLen] = '\0';

		char* m_char;
		int len = WideCharToMultiByte(CP_ACP, 0, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
		m_char = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, wszString, wcslen(wszString), m_char, len, NULL, NULL);
		m_char[len] = '\0';
		return m_char;
	}


	void UnicodeToUtf8(const wchar_t* unicode, char* src)
	{
		int len;
		len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
		//char *szUtf8 = new char[len+1];// (char*)malloc(len + 1);
		//memset(utf8_temp, 0, 1000);
		//memset(szUtf8, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, unicode, -1, src, len, NULL, NULL);
		//return utf8_temp;
	}
	std::string string_To_UTF8(const std::string& str)
	{
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;
		pwBuf = NULL;
		pBuf = NULL;
		return retStr;
	}

	bool is_str_utf8(const char* str)
	{
		unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
		unsigned char chr = *str;
		bool bAllAscii = true;
		for (unsigned int i = 0; str[i] != '\0'; ++i) {
			chr = *(str + i);
			//判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
			if (nBytes == 0 && (chr & 0x80) != 0) {
				bAllAscii = false;
			}
			if (nBytes == 0) {
				//如果不是ASCII码,应该是多字节符,计算字节数
				if (chr >= 0x80) {
					if (chr >= 0xFC && chr <= 0xFD) {
						nBytes = 6;
					}
					else if (chr >= 0xF8) {
						nBytes = 5;
					}
					else if (chr >= 0xF0) {
						nBytes = 4;
					}
					else if (chr >= 0xE0) {
						nBytes = 3;
					}
					else if (chr >= 0xC0) {
						nBytes = 2;
					}
					else {
						return false;
					}
					nBytes--;
				}
			}
			else {
				//多字节符的非首字节,应为 10xxxxxx
				if ((chr & 0xC0) != 0x80) {
					return false;
				}
				//减到为零为止
				nBytes--;
			}
		}
		//违返UTF8编码规则
		if (nBytes != 0) {
			return false;
		}
		if (bAllAscii) { //如果全部都是ASCII, 也是UTF8
			return true;
		}
		return true;
	}

	bool is_str_gbk(const char* str)
	{
		unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
		unsigned char chr = *str;
		bool bAllAscii = true; //如果全部都是ASCII,
		for (unsigned int i = 0; str[i] != '\0'; ++i) {
			chr = *(str + i);
			if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
				bAllAscii = false;
			}
			if (nBytes == 0) {
				if (chr >= 0x80) {
					if (chr >= 0x81 && chr <= 0xFE) {
						nBytes = +2;
					}
					else {
						return false;
					}
					nBytes--;
				}
			}
			else {
				if (chr < 0x40 || chr>0xFE) {
					return false;
				}
				nBytes--;
			}//else end
		}
		if (nBytes != 0) {   //违返规则
			return false;
		}
		if (bAllAscii) { //如果全部都是ASCII, 也是GBK
			return true;
		}
		return true;
	}


	/*************************************************************
 * 校验字符串是否以UTF-8编码,主要原理是判断位数是否对应
 * UTF-8编码规则:
 * 1字节 0xxxxxxx 注:和ascii码相同,这种不用判断,直接跳过
 * 2字节 110xxxxx 10xxxxxx
 * 3字节 1110xxxx 10xxxxxx 10xxxxxx
 * 4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 返回值 :0是utf8  其他不是utf8
 * ************************************************************/
	bool Is_Utf8_String(const char* utf, int length)
	{
		int check_sub = 0;
		int i = 0;
		int j = 0;

		for (i = 0; i < length; i++)
		{
			if (check_sub == 0)
			{
				if ((utf[i] >> 7) == 0) //0xxx xxxx
				{
					continue;
				}

				/* 获取单个 utf8 除了头部所占用字节个数 */
				struct
				{
					UINT8 cal;
					UINT8 cmp;
				} Utf8NumMap[] = {
					{0xE0,      0xC0},      //110xxxxx
					{0xF0,      0xE0},      //1110xxxx
					{0xF8,      0xF0},      //11110xxx
					{0xFC,      0xF8},      //111110xx
					{0xFE,      0xFC},      //1111110x
				};
				for (j = 0; j < (sizeof(Utf8NumMap) / sizeof(Utf8NumMap[0])); j++)
				{
					if ((utf[i] & Utf8NumMap[j].cal) == Utf8NumMap[j].cmp)
					{
						//printf("%u:%u:%x\n", __LINE__, i, utf[i]);
						check_sub = j + 1;
						break;
					}
				}
				if (0 == check_sub)
				{
					return false;
				}
			}
			else
			{
				/* 校验字节是否合法 */
				if ((utf[i] & 0xC0) != 0x80)
				{
					return 2;
				}
				check_sub--;
			}
		}
		return true;
	}

	//获取当前内存大小
	void  updateMemory(u32&  curvalue,u32& maxvalue)
	{
		HANDLE handle = GetCurrentProcess();
		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

		curvalue = pmc.WorkingSetSize;
		maxvalue = pmc.PeakWorkingSetSize;
	}

	//根据线路 地图ID 查找到游戏服务器所在的ID
	u32 findGameServerID(const u8 line, const u32 mapid)
	{
		if (line >= MAX_LINE_COUNT) return 0;
		if (mapid >= MAX_MAP_ID)    return 0;

		return __MapidToGameServerids[line][mapid];
	}


	
}
