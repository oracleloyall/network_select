
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include<netinet/ip.h>
#include <list>
#include<stdio.h>
#include<fcntl.h>
#define LOG_TAG "SocketListener"
#include<pthread.h>
#include "../include/SocketListener.h"
#include "../include/SocketClient.h"

#define CtrlPipe_Shutdown 0
#define CtrlPipe_Wakeup   1

SocketListener::SocketListener(const char *socketName, bool listen) {
    init(socketName, -1, listen, false);
}

SocketListener::SocketListener(int socketFd, bool listen) {
    init(NULL, socketFd, listen, false);
}

SocketListener::SocketListener(const char *socketName, bool listen, bool useCmdNum) {
    init(socketName, -1, listen, useCmdNum);
}

void SocketListener::init(const char *socketName, int socketFd, bool listen, bool useCmdNum) {
    mListen = listen;
    mSocketName = socketName;
    mSock = socketFd;
    mUseCmdNum = useCmdNum;
    pthread_mutex_init(&mClientsLock, NULL);
    mClients = new SocketClientCollection();
}

SocketListener::~SocketListener() {
    if (mSocketName && mSock > -1)
        close(mSock);

//    if (mCtrlPipe[0] != -1) {
//        close(mCtrlPipe[0]);
//        close(mCtrlPipe[1]);
//    }
    SocketClientCollection::iterator it;
    for (it = mClients->begin(); it != mClients->end();) {
        (*it)->decRef();
        it = mClients->erase(it);
    }
    delete mClients;
}

int SocketListener::startListener() {
    return startListener(4);
}

int SocketListener::startListener(int backlog) {


    if ( mSock == -1) {
       // printf("Failed to start unbound listener");
       // errno = EINVAL;
        mSock=0;
       // return -1;
    }
    	//android机制
//        if ((mSock = android_get_control_socket(mSocketName)) < 0) {
//            SLOGE("Obtaining file descriptor socket '%s' failed: %s",
//                 mSocketName, strerror(errno));
//            return -1;
//        }

    	struct sockaddr_in srv_addr;
    	memset(&srv_addr, 0, sizeof(struct sockaddr_in));
    	mSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    	if (mSock< 0) {
    			printf("Cannot create socket\n");
    			return 1;
    	}
    	srv_addr.sin_family = AF_INET;
    	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	srv_addr.sin_port = htons(8000);
        printf("get mSock = %d for %s\n", mSock, mSocketName);
        fcntl(mSock, F_SETFD, FD_CLOEXEC);
        if (bind(mSock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
        		printf("Cannot bind socket\n");
        		close(mSock);
        		return 1;
        	}

        int opt = 1;
        setsockopt(mSock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
       int listenfd=listen(mSock, 1);
        printf("listen fd is %d \n",listenfd);
    if (mListen && listenfd< 0) {
    	printf("Unable to listen on socket\n");
        return -1;
    } else if (!mListen)
    {
        mClients->push_back(new SocketClient(mSock, false, mUseCmdNum));
    }

//    if (pipe(mCtrlPipe)) {
//        SLOGE("pipe failed (%s)", strerror(errno));
//        return -1;
//    }

    if (pthread_create(&mThread, NULL, SocketListener::threadStart, this)) {
        SLOGE("pthread_create (%s)", strerror(errno));
        return -1;
    }

    return 0;
}

int SocketListener::stopListener() {
    char c = CtrlPipe_Shutdown;
    int  rc;

//    rc = TEMP_FAILURE_RETRY(write(mCtrlPipe[1], &c, 1));
//    if (rc != 1) {
//        SLOGE("Error writing to control pipe (%s)", strerror(errno));
//        return -1;
//    }

    void *ret;
    if (pthread_join(mThread, &ret)) {
        SLOGE("Error joining to listener thread (%s)", strerror(errno));
        return -1;
    }
//    close(mCtrlPipe[0]);
//    close(mCtrlPipe[1]);
//    mCtrlPipe[0] = -1;
//    mCtrlPipe[1] = -1;

    if (mSocketName && mSock > -1) {
        close(mSock);
        mSock = -1;
    }

    SocketClientCollection::iterator it;
    for (it = mClients->begin(); it != mClients->end();) {
        delete (*it);
        it = mClients->erase(it);
    }
    return 0;
}

void *SocketListener::threadStart(void *obj) {

    SocketListener *me = reinterpret_cast<SocketListener *>(obj);

    me->runListener();
    pthread_exit(NULL);
    return NULL;
}

void SocketListener::runListener() {


    SocketClientCollection pendingList;

    while(1) {
        SocketClientCollection::iterator it;
        fd_set read_fds;
        int rc = 0;
        int max = -1;

        FD_ZERO(&read_fds);

        if (mListen) {
            max = mSock;
            FD_SET(mSock, &read_fds);
        }

//        FD_SET(mCtrlPipe[0], &read_fds);
//        if (mCtrlPipe[0] > max)
//            max = mCtrlPipe[0];
        printf("into runlistenter\n");
        pthread_mutex_lock(&mClientsLock);
        for (it = mClients->begin(); it != mClients->end(); ++it) {
            // NB: calling out to an other object with mClientsLock held (safe)
            int fd = (*it)->getSocket();
            FD_SET(fd, &read_fds);
            if (fd > max) {
                max = fd;
            }
        }
        pthread_mutex_unlock(&mClientsLock);

       // printf("mListen=%d, max=%d, mSocketName=%s", mListen, max, mSocketName);
        if ((rc = select(max + 1, &read_fds, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
            {
                printf("error EINTR\n");
                continue;
            }
         printf("select failed \n ");
            sleep(1);
            continue;
        } else if (!rc)
            continue;

//        if (FD_ISSET(mCtrlPipe[0], &read_fds)) {
//            char c = CtrlPipe_Shutdown;
//            TEMP_FAILURE_RETRY(read(mCtrlPipe[0], &c, 1));
//            if (c == CtrlPipe_Shutdown) {
//                break;
//            }
//            continue;
//        }
        if (mListen && FD_ISSET(mSock, &read_fds)) {
            struct sockaddr addr;
            socklen_t alen;
            int c;

            do {
                alen = sizeof(addr);
                c = accept(mSock, &addr, &alen);
                printf("%s got %d from accept\n", mSocketName, c);
            } while (c < 0 && errno == EINTR);
            if (c < 0) {
                printf("accept failed (%s)", strerror(errno));
                sleep(1);
                continue;
            }
            fcntl(c, F_SETFD, FD_CLOEXEC);
            pthread_mutex_lock(&mClientsLock);
            printf("put socket into server \n");
            mClients->push_back(new SocketClient(c, true, mUseCmdNum));
            pthread_mutex_unlock(&mClientsLock);
        }

        /* Add all active clients to the pending list first */
        pendingList.clear();
        pthread_mutex_lock(&mClientsLock);
        for (it = mClients->begin(); it != mClients->end(); ++it) {
            SocketClient* c = *it;
            // NB: calling out to an other object with mClientsLock held (safe)
            int fd = c->getSocket();
            if (FD_ISSET(fd, &read_fds)) {
                pendingList.push_back(c);
                c->incRef();
            }
        }
        pthread_mutex_unlock(&mClientsLock);

        /* Process the pending list, since it is owned by the thread,
         * there is no need to lock it */
        while (!pendingList.empty()) {
            /* Pop the first item from the list */
            it = pendingList.begin();
            SocketClient* c = *it;
            pendingList.erase(it);
            /* Process it, if false is returned, remove from list */
            if (!onDataAvailable(c)) {
                release(c, false);
            }
            c->decRef();
        }
    }
}

bool SocketListener::release(SocketClient* c, bool wakeup) {
    bool ret = false;
    /* if our sockets are connection-based, remove and destroy it */
    if (mListen && c) {
        /* Remove the client from our array */
        SLOGV("going to zap %d for %s", c->getSocket(), mSocketName);
        pthread_mutex_lock(&mClientsLock);
        SocketClientCollection::iterator it;
        for (it = mClients->begin(); it != mClients->end(); ++it) {
            if (*it == c) {
                mClients->erase(it);
                ret = true;
                break;
            }
        }
        pthread_mutex_unlock(&mClientsLock);
//        if (ret) {
//            ret = c->decRef();
//            if (wakeup) {
//                char b = CtrlPipe_Wakeup;
//                TEMP_FAILURE_RETRY(write(mCtrlPipe[1], &b, 1));
//            }
//        }
    }
    return ret;
}

void SocketListener::sendBroadcast(int code, const char *msg, bool addErrno) {
    SocketClientCollection safeList;

    /* Add all active clients to the safe list first */
    safeList.clear();
    pthread_mutex_lock(&mClientsLock);
    SocketClientCollection::iterator i;

    for (i = mClients->begin(); i != mClients->end(); ++i) {
        SocketClient* c = *i;
        c->incRef();
        safeList.push_back(c);
    }
    pthread_mutex_unlock(&mClientsLock);

    while (!safeList.empty()) {
        /* Pop the first item from the list */
        i = safeList.begin();
        SocketClient* c = *i;
        safeList.erase(i);
        // broadcasts are unsolicited and should not include a cmd number
        if (c->sendMsg(code, msg, addErrno, false)) {
            printf("Error sending broadcast (%s)", strerror(errno));
        }
        c->decRef();
    }
}

void SocketListener::runOnEachSocket(SocketClientCommand *command) {
    SocketClientCollection safeList;

    /* Add all active clients to the safe list first */
    safeList.clear();
    pthread_mutex_lock(&mClientsLock);
    SocketClientCollection::iterator i;

    for (i = mClients->begin(); i != mClients->end(); ++i) {
        SocketClient* c = *i;
        c->incRef();
        safeList.push_back(c);
    }
    pthread_mutex_unlock(&mClientsLock);

    while (!safeList.empty()) {
        /* Pop the first item from the list */
        i = safeList.begin();
        SocketClient* c = *i;
        safeList.erase(i);
        command->runSocketCommand(c);
        c->decRef();
    }
}
