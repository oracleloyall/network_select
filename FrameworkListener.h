
#ifndef _FRAMEWORKSOCKETLISTENER_H
#define _FRAMEWORKSOCKETLISTENER_H

#include "SocketListener.h"
#include "FrameworkCommand.h"

class SocketClient;

class FrameworkListener : public SocketListener {
public:
    static const int CMD_ARGS_MAX = 26;

    /* 1 out of errorRate will be dropped */
    int errorRate;

private:
    int mCommandCount;
    bool mWithSeq;
    FrameworkCommandCollection *mCommands;

public:
    FrameworkListener(const char *socketName);
    FrameworkListener(const char *socketName, bool withSeq);
    FrameworkListener(int sock);
    virtual ~FrameworkListener() {}

protected:
    void registerCmd(FrameworkCommand *cmd);
    virtual bool onDataAvailable(SocketClient *c);

private:
    void dispatchCommand(SocketClient *c, char *data);
    void init(const char *socketName, bool withSeq);
};
#endif
