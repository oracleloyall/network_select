
#ifndef __FRAMEWORK_CMD_HANDLER_H
#define __FRAMEWORK_CMD_HANDLER_H

#include "List.h"

class SocketClient;

class FrameworkCommand { 
private:
    const char *mCommand;

public:

    FrameworkCommand(const char *cmd);
    virtual ~FrameworkCommand() { }

    virtual int runCommand(SocketClient *c, int argc, char **argv) = 0;

    const char *getCommand() { return mCommand; }
};

typedef android::sysutils::List<FrameworkCommand *> FrameworkCommandCollection;
#endif
