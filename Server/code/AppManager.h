#ifndef  ____APPMANAGER_H
#define  ____APPMANAGER_H

#ifdef ____WIN32_
//#pragma comment(lib,"libs/myengine.lib") 
#endif

namespace app
{

	class AppManager
	{
	public:
		AppManager();
		~AppManager();
		void  init();
	};
	extern int run();
}


#endif