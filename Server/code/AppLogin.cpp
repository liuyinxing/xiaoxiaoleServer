#include "AppLogin.h"
#include "UserManager.h"
#include "ShareFunction.h"
#include "UserData.h"
#include <random>
#include "IOPool.h"
using namespace func;
namespace app
{
	AppLogin* __AppLogin = NULL;
	std::default_random_engine kingRandom(static_cast<unsigned int>(time(NULL)));
	void onLoginGame(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
	void onHeart(net::ITcpServer* ts, net::S_CLIENT_BASE* c);

	AppLogin::AppLogin(){}
	AppLogin::~AppLogin(){}
	void AppLogin::onInit(){}

	void checkOnlineUserNotInGame();
	void AppLogin::onUpdate()
	{
		checkOnlineUserNotInGame();
	}
	



	//收到客户端发送上来的指令数据
	bool AppLogin::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_ConnectSecure))
		{
			LOG_MSG("AppLogin err...line:%d \n", __LINE__);
			return false;
		}
		switch (cmd)
		{
		case CMD_HEART:onHeart(ts, c);  break;      //玩家心跳包
		case CMD_100:onLoginGame(ts, c); break;
		}
		return true;
	}

	bool AppLogin::onTimerMessage(void* buff, UINT byteSize)
	{
		DataContext* tm_context = (DataContext*)buff;

		LOG_MSG("APP_login定时器触发 %d %d ", tm_context->m_Types, tm_context->timerId);
		__TcpServer->KillTimer(tm_context->timerId);
		return true;
	}

	//100 角色名注册
	void onLoginGame(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		auto user = __UserManager->findFreeUser();
		if (nullptr == user)
		{
	//		ts->begin(c->ID, CMD_100);
	//		ts->sss(c->ID, u8(0));
	//		ts->end(c->ID);
			LOG_MSG("玩家在线已满:%d \n", __LINE__);
			return;
		}
		char name[USER_MAX_MEMBER];
	//	ts->read(c->ID,name, USER_MAX_MEMBER);
		memcpy(user->name, &name, USER_MAX_MEMBER);
		user->state = 1;
		user->id = c->ID;
	//	ts->begin(c->ID, CMD_100);
	//	ts->sss(c->ID, u8(1));
		
		ts->end(c->ID);
	}



	//60000 heart
	void onHeart(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		S_USER_GATE* user = __UserManager->findUser(c->ID);
		if (user == nullptr)  return;

	}

	s32 RandomRange(u32 min, u32 max)//[min max]
	{
		s32 num = max - min;
		if (num <= 0) return 0;
		return  rand() % (num + 1) + min;
	}


	s32 RandomUniform(u32 min, u32 max)//[min max]
	{
		s32 num = max - min;
		if (num <= 0) return 0;
		std::uniform_int_distribution<int> u(min, max);
		return  u(kingRandom);
	}


	void myRandom(int* cells)
	{
		int i = 0;
		for (;i < GAME_CELLS; ++i)
		{
			cells[i] = RandomUniform(0,6);
		}
	}

	void checkOnlineUserNotInGame()
	{
		S_USER_GATE* temp=nullptr;
		srand(time(NULL));
		int count = __UserManager->__OnlineUsers->length;
		for (int i = 0; i < count; ++i)
		{
			auto user = __UserManager->__OnlineUsers->Value(i);
			if (1 != user->state) continue;
			if (nullptr == temp)
			{
				temp = user;
				continue;
			}
			__UserManager->InserGameMap(temp, user);
			myRandom(temp->cells);
			myRandom(user->cells);
			temp->state = 2;
			user->state = 2;
	/*		__TcpServer->begin(temp->id, CMD_200);
			__TcpServer->sss(temp->id, temp->cells, sizeof(s32) * GAME_CELLS);
			__TcpServer->sss(temp->id, user, sizeof(S_USER_GATE));
			__TcpServer->end(temp->id);
			__TcpServer->begin(user->id, CMD_200);
			__TcpServer->sss(user->id, user->cells, sizeof(s32) * GAME_CELLS);
			__TcpServer->sss(user->id, temp, sizeof(S_USER_GATE));
			__TcpServer->end(user->id);*/
			
			
		}
	}

}

