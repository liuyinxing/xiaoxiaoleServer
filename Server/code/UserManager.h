#ifndef ____USER_MANAGER_H
#define ____USER_MANAGER_H

#include  "AppGlobal.h"
#include  "UserData.h"
#include  "WorldData.h"
#include <concurrent_queue.h>
#include <unordered_map>
namespace app
{

	class UserManager
	{
	public:
		HashArray<S_USER_GATE>* __OnlineUsers;
		std::unordered_map < S_USER_GATE* , S_USER_GATE*> __InGameMap;
		Concurrency::concurrent_queue<u8>  __TcpClientDisconnets;//���߶���
	public:
		UserManager();
		~UserManager();

		void init();
		void update();//����
		S_USER_GATE* findFreeUser();
		S_USER_GATE* findNotInGameUser();
		S_USER_GATE* findUser(const u32 connectid);
		bool InserGameMap(S_USER_GATE* _first, S_USER_GATE* _second);
		net::ITcpClient* findGameServer_Connection(u8 line,u32 mapid);
		inline void setTcpClientDisconnect(u8 type)
		{
			__TcpClientDisconnets.push(type);
		}
	};
	extern UserManager* __UserManager;//�û����ݹ�����
}


#endif
