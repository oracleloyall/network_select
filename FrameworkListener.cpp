
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include<stdio.h>
#define LOG_TAG "FrameworkListener"



#include "../include/FrameworkListener.h"
#include "../include/FrameworkCommand.h"
#include "../include/SocketClient.h"

static const int CMD_BUF_SIZE = 1024;

#define UNUSED __attribute__((unused))

FrameworkListener::FrameworkListener(const char *socketName, bool withSeq) :
                            SocketListener(socketName, true, withSeq) {
    init(socketName, withSeq);
}

FrameworkListener::FrameworkListener(const char *socketName) :
                            SocketListener(socketName, true, false) {
    init(socketName, false);
}

FrameworkListener::FrameworkListener(int sock) :
                            SocketListener(sock, true) {
    init(NULL, false);
}

void FrameworkListener::init(const char *socketName UNUSED, bool withSeq) {
    mCommands = new FrameworkCommandCollection();
    errorRate = 0;
    mCommandCount = 0;
    mWithSeq = withSeq;
}
int total=0;
bool FrameworkListener::onDataAvailable(SocketClient *c) {
//这个函数解析包，然后处理
    char buffer[CMD_BUF_SIZE];
    int len;

    len = TEMP_FAILURE_RETRY(read(c->getSocket(), buffer, sizeof(buffer)));
    if (len < 0) {
        printf("read() failed (%s)", strerror(errno));
        return false;
    } else if (!len)
        return false;
    printf("get %s total =%d\n",buffer,total++);
    write(c->getSocket(),"send by zhaoxi \n",100);
#ifdef MESSAGE
   if(buffer[len-1] != '\0')
   {
	   buffer[len-1]='\0';
       printf("String is not zero-terminated");
   }

    int offset = 0;
    int i;

    for (i = 0; i < len; i++) {
        if (buffer[i] == '\0') {
            /* IMPORTANT: dispatchCommand() expects a zero-terminated string */
        	printf("dispatch\n");
            dispatchCommand(c, buffer + offset);
            offset = i + 1;
        }
    }
#endif
    return true;
}

void FrameworkListener::registerCmd(FrameworkCommand *cmd) {
    mCommands->push_back(cmd);
}

void FrameworkListener::dispatchCommand(SocketClient *cli, char *data) {
    FrameworkCommandCollection::iterator i;
    int argc = 0;
    char *argv[FrameworkListener::CMD_ARGS_MAX];
    char tmp[CMD_BUF_SIZE];
    char *p = data;
    char *q = tmp;
    char *qlimit = tmp + sizeof(tmp) - 1;
    bool esc = false;
    bool quote = false;
    bool haveCmdNum = !mWithSeq;

    memset(argv, 0, sizeof(argv));
    memset(tmp, 0, sizeof(tmp));
    while(*p) {
        if (*p == '\\') {
            if (esc) {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '\\';
                esc = false;
            } else
                esc = true;
            p++;
            continue;
        } else if (esc) {
            if (*p == '"') {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '"';
            } else if (*p == '\\') {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '\\';
            } else {
                cli->sendMsg(500, "Unsupported escape sequence", false);
                goto out;
            }
            p++;
            esc = false;
            continue;
        }

        if (*p == '"') {
            if (quote)
                quote = false;
            else
                quote = true;
            p++;
            continue;
        }

        if (q >= qlimit)
            goto overflow;
        *q = *p++;
        if (!quote && *q == ' ') {
            *q = '\0';
            if (!haveCmdNum) {
                char *endptr;
                int cmdNum = (int)strtol(tmp, &endptr, 0);
                if (endptr == NULL || *endptr != '\0') {
                    cli->sendMsg(500, "Invalid sequence number", false);
                    goto out;
                }
                cli->setCmdNum(cmdNum);
                haveCmdNum = true;
            } else {
                if (argc >= CMD_ARGS_MAX)
                    goto overflow;
                argv[argc++] = strdup(tmp);
            }
            memset(tmp, 0, sizeof(tmp));
            q = tmp;
            continue;
        }
        q++;
    }

    *q = '\0';
    if (argc >= CMD_ARGS_MAX)
        goto overflow;
    argv[argc++] = strdup(tmp);
#if 0
    for (int k = 0; k < argc; k++) {
        SLOGD("arg[%d] = '%s'", k, argv[k]);
    }
#endif

    if (quote) {
        cli->sendMsg(500, "Unclosed quotes error", false);
        goto out;
    }

    if (errorRate && (++mCommandCount % errorRate == 0)) {
        /* ignore this command - let the timeout handler handle it */
        SLOGE("Faking a timeout");
        goto out;
    }

    for (i = mCommands->begin(); i != mCommands->end(); ++i) {
        FrameworkCommand *c = *i;

        if (!strcmp(argv[0], c->getCommand())) {
            if (c->runCommand(cli, argc, argv)) {
                printf("Handler '%s' error (%s)", c->getCommand(), strerror(errno));
            }
            goto out;
        }
    }
    cli->sendMsg(500, "Command not recognized", false);
out:
    int j;
    for (j = 0; j < argc; j++)
        free(argv[j]);
    return;

overflow:
   // LOG_EVENT_INT(78001, cli->getUid());
    cli->sendMsg(500, "Command too long", false);
    goto out;
}
