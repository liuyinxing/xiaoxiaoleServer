#ifndef  __APPROOM_H
#define __APPROOM_H
#include "IContainer.h"
#include "UserData.h"
#include <atltime.h>
#define GFF_FORCE_FINISH			0										///< 强行解除
#define GFF_SAFE_FINISH				1										///< 解除游戏

///开始模式
#define FULL_BEGIN					0x00									///< 满人才开始
#define ALL_ARGEE					0x01									///< 所有人同意就开始
#define SYMMETRY_ARGEE				0x02									///< 对称同意开始
#define FOUR_SIX_ARGEE				0x03									///< 6个座位中只要有4个人同意开始
#define MAX_PEOPLE                  0x02
///定时器宏
#define TIME_SPACE						50L									///游戏 ID 间隔
#define TIME_START_ID					100L								///定时器开始 ID

namespace app
{
	class RoomManage;
	class Room
	{

	public:
		s32					roomid;						//房间ID
		u8					m_bMaxPeople;							///< 游戏人数
		const u8			m_bBeginMode;				///< 同意模式
		S_USER_GATE			* m_pUserInfo[MAX_PEOPLE];	//房间玩家数据
		RoomManage			* m_pRoomMange;				//房间管理指针
		CTime				m_tBeginTime;				///游戏开始的时间
		CTime				m_tEndTime;					///游戏结束的时间
		bool				m_bPlayGame;				///< 游戏是否开始标志
		long int			m_dwBeginTime;							///< 开始时间
	public:
		explicit Room(u8 beginMode);
		virtual ~Room() {};
		bool Init(u8 _roomid, RoomManage* pRoomManage);
		bool SetTimer(UINT uTimerID, int uElapse);
		bool KillTimer(UINT uTimerID);
		long int GetPlayTimeCount();
		bool UserAgreeGame(BYTE bDeskStation);
	};

	class RoomManage
	{
	public:
		RoomManage();
		~RoomManage();

	private:
		HWND				m_hWindow;					///窗口句柄
	public:
		HashArray<Room>* __RoomList;
		void init();
		Room* FindFreeRoom();
		Room* FindRoom(s32 room_id);
		void DeleteRoom(s32 room_id);
		//设定定时器
		bool SetTimer(UINT uTimerID, UINT uElapse);

		bool KillTimer(UINT uTimerID);
	
	};

	extern RoomManage* __RoomManager;//房间管理器
}

#endif //  __APPROOM_H
