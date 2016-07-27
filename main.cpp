#include<stdio.h>
#include<string>
#include"../include/List.h"
#include"../include/CommandListener.h"
#include"../include/FrameworkClient.h"
#include"../include/FrameworkCommand.h"
#include"../include/FrameworkListener.h"
#include"../include/SocketClient.h"
#include"../include/SocketClientCommand.h"
#include"../include/SocketListener.h"
#include"../include/VoldCommand.h"

using namespace std;

typedef android::sysutils::List<string *> lis;
int main(int argc,char ** argv)
{
	CommandListener *cl;
	cl = new CommandListener();
	  if (cl->startListener()) {
	        perror("unable to start listen");
	        exit(1);
	    }
	while(1) {
	        sleep(1000);
	    }
	return 0;
}
