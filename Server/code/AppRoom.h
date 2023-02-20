#ifndef  __APPROOM_H
#define __APPROOM_H
#include "IContainer.h"
#include "UserData.h"
#include <atltime.h>
#define GFF_FORCE_FINISH			0										///< ǿ�н��
#define GFF_SAFE_FINISH				1										///< �����Ϸ

///��ʼģʽ
#define FULL_BEGIN					0x00									///< ���˲ſ�ʼ
#define ALL_ARGEE					0x01									///< ������ͬ��Ϳ�ʼ
#define SYMMETRY_ARGEE				0x02									///< �Գ�ͬ�⿪ʼ
#define FOUR_SIX_ARGEE				0x03									///< 6����λ��ֻҪ��4����ͬ�⿪ʼ
#define MAX_PEOPLE                  0x02
///��ʱ����
#define TIME_SPACE						50L									///��Ϸ ID ���
#define TIME_START_ID					100L								///��ʱ����ʼ ID

namespace app
{
	class RoomManage;
	class Room
	{

	public:
		s32					roomid;						//����ID
		u8					m_bMaxPeople;							///< ��Ϸ����
		const u8			m_bBeginMode;				///< ͬ��ģʽ
		S_USER_GATE			* m_pUserInfo[MAX_PEOPLE];	//�����������
		RoomManage			* m_pRoomMange;				//�������ָ��
		CTime				m_tBeginTime;				///��Ϸ��ʼ��ʱ��
		CTime				m_tEndTime;					///��Ϸ������ʱ��
		bool				m_bPlayGame;				///< ��Ϸ�Ƿ�ʼ��־
		long int			m_dwBeginTime;							///< ��ʼʱ��
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
		HWND				m_hWindow;					///���ھ��
	public:
		HashArray<Room>* __RoomList;
		void init();
		Room* FindFreeRoom();
		Room* FindRoom(s32 room_id);
		void DeleteRoom(s32 room_id);
		//�趨��ʱ��
		bool SetTimer(UINT uTimerID, UINT uElapse);

		bool KillTimer(UINT uTimerID);
	
	};

	extern RoomManage* __RoomManager;//���������
}

#endif //  __APPROOM_H
