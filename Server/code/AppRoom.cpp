#include "AppRoom.h"

namespace app
{
	RoomManage* __RoomManager = nullptr;
	RoomManage::RoomManage()
	{
		__RoomList = nullptr;
		init();
	}
	RoomManage::~RoomManage()
	{
	}
	void RoomManage::init()
	{
		//1、创建房间数据
		__RoomList = new HashArray<Room>(ROOM_SIZE);
		for (int i = 0; i < __RoomList->length; i++)
		{
			Room* user = __RoomList->Value(i);
			//user->reset();
		}
	}

	//查找空闲房间
	Room * RoomManage::FindFreeRoom()
	{
		for (int i = 0; i < __RoomList->length; i++)
		{
			Room* room = __RoomList->Value(i);
			if (room->roomid != 0)continue;
			return room;
		}
		return nullptr;
	}
	//查找房间
	Room* RoomManage::FindRoom(s32 room_id)
	{
		if (room_id < 1  && room_id >= ROOM_SIZE) return nullptr;
		Room* room = __RoomList->Value(room_id);
		if (room->roomid == 0) return nullptr;
		return room;
	}
	//玩家退出游戏 删除房间
	void RoomManage::DeleteRoom(s32 room_id)
	{
		if (room_id <1 && room_id>=ROOM_SIZE) return ;
		Room* room = FindRoom(room_id);
		if (nullptr != room)
		{
			//room->reset();
		}
	}
	///用户同意开始
	bool UserAgreeGame(BYTE bDeskStation)
	{
		return 0;
	};

	bool RoomManage::SetTimer(UINT uTimerID, UINT uElapse)
	{
			if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
			{
				::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
				return true;
			}
			return false;
	}

	//清除定时器
	bool RoomManage::KillTimer(UINT uTimerID)
	{
		if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
		{
			::KillTimer(m_hWindow, uTimerID);
			return true;
		}
		return false;
	}

	Room::Room(u8 beginMode): m_bBeginMode(beginMode)
	{
		roomid = 0;
		m_bMaxPeople = 0;
		memset(m_pUserInfo, 0, sizeof(S_USER_GATE* )*MAX_PEOPLE);
		m_pRoomMange = nullptr;
		m_tBeginTime = (long int)time(NULL);
		m_tEndTime = 0;
	}

	bool Room::Init(u8 _roomid, RoomManage* pRoomManage)
	{
		roomid = _roomid;
		m_pRoomMange = pRoomManage;
		return true;
	}

	bool Room::SetTimer(UINT uTimerID, int uElapse)
	{
		if (uTimerID >= TIME_SPACE) return false;
		return m_pRoomMange->SetTimer(roomid * TIME_SPACE + uTimerID + TIME_START_ID, uElapse);
	}

	bool Room::KillTimer(UINT uTimerID)
	{
		if (uTimerID >= TIME_SPACE) return false;
		return m_pRoomMange->KillTimer(roomid * TIME_SPACE + TIME_START_ID + uTimerID);
	}

	long int Room::GetPlayTimeCount()
	{
		if (m_bPlayGame == false) return 0L;
		return (long int)time(NULL) - m_dwBeginTime;
	}

	bool Room::UserAgreeGame(BYTE bDeskStation)
	{
		return false;
	}




}

