
#ifndef _SOCKETCLIENTCOMMAND_H
#define _SOCKETCLIENTCOMMAND_H

#include "SocketClient.h"

class SocketClientCommand {
public:
    virtual ~SocketClientCommand() { }
    virtual void runSocketCommand(SocketClient *client) = 0;
};

#endif
