#include "AppManager.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "LogFile.h"
#include "AppLogin.h"
#include "UserManager.h"
#ifndef  ____WIN32_
#include <unistd.h>
#endif



namespace app
{
	AppManager* __AppManager = nullptr;
	int temp_time = 0;
	char printfstr[1000];

	AppManager::AppManager()
	{
	}

	AppManager::~AppManager()
	{
	}


	//打印信息
	void printInfo()
	{
#ifdef  ____WIN32_
		int tempTime = (int)time(NULL) - temp_time;
		if (tempTime < 1) return;

		int concount = 0;
		int securtiycount = 0;
		__TcpServer->getSecurityCount(concount, securtiycount);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		int concount2 = 0;

		sprintf_s(printfstr, "[Gate:%d id:%d] connect-%d  security-%d [memory:%.4f/%.4f] udp:%d", 
			func::__ServerInfo->Port, func::__ServerInfo->ID,concount, securtiycount,
			curmemory1, maxmemory1, concount2);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;
		__TcpServer->parseCommand();
		if (__UserManager != nullptr) __UserManager->update();
		printInfo();
		if (__AppLogin != nullptr) __AppLogin->onUpdate();

	}

	void AppManager::init()
	{

		bool isload = share::InitData(true);
		if (!isload) return;
	//	if (func::__ServerListInfo.size() < 1) return;

		io::runLogThread();

		//创建服务器 启动
		__TcpServer = net::NewTcpServer();
		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisconnect(onClientDisconnect);
		__TcpServer->setOnClientTimeout(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(1);
		
		//创建对象
		__UserManager = new UserManager();
		__AppLogin = new AppLogin();
		//__RoomManager = new RoomManage();
		//服务器
		__TcpServer->registerCommand(111, __AppLogin);
		__TcpServer->registerCommand(CMD_HEART, __AppLogin);
		__TcpServer->registerCommand(CMD_100, __AppLogin);//获取角色基础数据
		__TcpServer->registerTimerContainer(__AppLogin);
		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);
		io::pushLog(io::EFT_RUN, "runServer success:%d %d memory:%.4f/%.4f\n",
			func::__ServerInfo->Port, func::__ServerInfo->ID, curmemory1, maxmemory1);
		io::pushLog(io::EFT_ERR, "runServer success:%d %d memory:%.4f/%.4f\n",
			func::__ServerInfo->Port, func::__ServerInfo->ID, curmemory1, maxmemory1);

		while (true)
		{
			onUpdate();
#ifdef  ____WIN32_
			Sleep(2);
#else
			usleep(2);
#endif
		}

	}
	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}

		return 0;
	}
}

