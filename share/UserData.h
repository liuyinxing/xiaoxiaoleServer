#ifndef ___USERDATA_H
#define ___USERDATA_H
#include <IDefine.h>
#include <string>
#include <string.h>
//#define TEST_UDP_SERVER 1
#include<vector>

#define  USER_MAX_MEMBER_CHECK    21
#define  USER_MAX_MEMBER          20
#define  USER_MAX_PASS            20
#define  USER_MAX_KEY             11
#define  USER_MAX_NICK            20
#define  USER_MAX_ROLE            3
#define  GAME_CELLS               36
#define  CELLS_ROW				  6
#define  CELLS_COM				  6
#define  CELLS_COINS			  6
#define  ROOM_SIZE				100
namespace app
{
#pragma pack(push, packing)
#pragma pack(1)


	//�û�����
	struct  S_USER_GATE
	{
		u8   state; //��ǰ״̬  1ע��ɹ� �ȴ����    2 ��Ϸ��  0 �뿪��Ϸ
		char name[USER_MAX_MEMBER];
		s32	 cells[GAME_CELLS];
		s32  cellsRuslat[GAME_CELLS];
		s32  id;
		s32	 userIndex;
		long int	memid;							///ID ����
		__int64		i64Money;		///���
		int			dwPoint;							///��ǰ����
		u8			room_id;		//�����
		char		nickName[61];						///�û��ǳ�
		int			userType;		//�û�����  0 ��� 1����Ա
		S_USER_GATE()
		{
			ZeroMemory(this, sizeof(S_USER_GATE));
		}
	};
#pragma pack(pop, packing)
}

#endif