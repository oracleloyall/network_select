
#ifndef _SOCKETLISTENER_H
#define _SOCKETLISTENER_H

#include <pthread.h>
#define SLOGV(...) printf(__VA_ARGS__)
#define SLOGE(...) printf(__VA_ARGS__)
#include "SocketClient.h"
#include "SocketClientCommand.h"

class SocketListener {
    bool                    mListen;
    const char              *mSocketName;
    int                     mSock;
    SocketClientCollection  *mClients;
    pthread_mutex_t         mClientsLock;
  //  int                     mCtrlPipe[2];
    pthread_t               mThread;
    bool                    mUseCmdNum;

public:
    SocketListener(const char *socketName, bool listen);
    SocketListener(const char *socketName, bool listen, bool useCmdNum);
    SocketListener(int socketFd, bool listen);

    virtual ~SocketListener();
    int startListener();
    int startListener(int backlog);
    int stopListener();

    void sendBroadcast(int code, const char *msg, bool addErrno);

    void runOnEachSocket(SocketClientCommand *command);

    bool release(SocketClient *c) { return release(c, true); }

protected:
    virtual bool onDataAvailable(SocketClient *c) = 0;

private:
    bool release(SocketClient *c, bool wakeup);
    static void *threadStart(void *obj);
    void runListener();
    void init(const char *socketName, int socketFd, bool listen, bool useCmdNum);
};
#endif
