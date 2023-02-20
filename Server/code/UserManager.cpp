#include "UserManager.h"
#include "CommandData2.h"
#include "ShareFunction.h"
#include "UserData.h"
#include <LogFile.h>

namespace app
{
	UserManager* __UserManager = nullptr;


	UserManager::UserManager()
	{
		__OnlineUsers = NULL;
		init();
	}

	UserManager::~UserManager()
	{
	}
	void UserManager::init()
	{
		//1、创建连接用户的 玩家游戏基础数据
		__OnlineUsers = new HashArray<S_USER_GATE>(func::__ServerInfo->MaxConnect);
		for (int i = 0; i < __OnlineUsers->length; i++)
		{
			S_USER_GATE* user = __OnlineUsers->Value(i);
			//user->reset();
		}
	}

	void UserManager::update()
	{
		//1、处理当前做为客户端 掉线的情况
		if (!__TcpClientDisconnets.empty())
		{
			int num = 0;
			while (true)
			{
				u8 server_type = 0;
				__TcpClientDisconnets.try_pop(server_type);

				switch (server_type)
				{
				case func::S_TYPE_GAME:
					break;
				//case func::S_TYPE_TEAM:
				//	//onCheckDisconnect_Team();
				//	break;
				}
				if (__TcpClientDisconnets.empty()) break;
				//避免死循环
				++num;
				if (num >= 10) break;
			}
		}

	}
	S_USER_GATE* UserManager::findFreeUser()
	{
		int count = __OnlineUsers->length;
		for (int i = 0; i < count; i++)
		{
			auto user = __OnlineUsers->Value(i);
			if (user == nullptr) continue;
			if (user->state != 0) continue;
			//user->reset();
			return user;
		}
		return nullptr;
	}

	S_USER_GATE* UserManager::findNotInGameUser()
	{
		int count = __OnlineUsers->length;
		for (int i = 0; i < count; i++)
		{
			auto user = __OnlineUsers->Value(i);
			if (user == nullptr) continue;
			if (user->state !=1) continue;
			user->userIndex = i;
			return user;
		}
		return nullptr;
	}

	//查找玩家
	S_USER_GATE* UserManager::findUser(const u32 userindex)
	{
		if (userindex >= __OnlineUsers->length) return nullptr;
		return __OnlineUsers->Value(userindex);
	}
	////查找玩家
	//S_USER_GATE* UserManager::findUser(const u32 userindex, const u64 mid)
	//{
	//	auto user = findUser(userindex);
	//	if (user == NULL) return user;
	//	return user;
	//}



	net::ITcpClient* UserManager::findGameServer_Connection(u8 line, u32 mapid)
	{
		return nullptr;
	}

	bool UserManager::InserGameMap(S_USER_GATE* _first, S_USER_GATE* _second)
	{
		auto it = __InGameMap.find(_first);
		if (it != __InGameMap.end())
		{
			__InGameMap.insert(std::make_pair(_first, _second));
			return true;
		}
		return false;
	}

	//********************************************************************
	//********************************************************************
	//********************************************************************

}

