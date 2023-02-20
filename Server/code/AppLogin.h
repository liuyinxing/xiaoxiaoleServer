#ifndef  ____APPLOGIN_H
#define  ____APPLOGIN_H 

#include "IContainer.h"
#include "CommandData.h"
namespace app
{

	class AppLogin : public IContainer
	{
	public:
		AppLogin();
		virtual ~AppLogin();
		virtual void  onInit();
		virtual void  onUpdate();
		virtual bool  onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onTimerMessage(void* buff, UINT byteSize);
	};

	extern AppLogin* __AppLogin;
}

#endif