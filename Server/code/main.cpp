#include <cstdio>

#include "../../share/ShareFunction.h"
#include <unistd.h>
#include "AppManager.h"
#include <signal.h>
int main()
{
	signal(SIGPIPE, SIG_IGN);
    printf("hello from testlinux!\n");
	app::run();
	//share::InitData();
	//usleep(10000 * 1000);
    return 0;
}