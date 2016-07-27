
#include <errno.h>
#include<stdio.h>
#define LOG_TAG "FrameworkCommand"


#include "../include/FrameworkCommand.h"

#define UNUSED __attribute__((unused))

FrameworkCommand::FrameworkCommand(const char *cmd) {
    mCommand = cmd;
}

int FrameworkCommand::runCommand(SocketClient *c UNUSED, int argc UNUSED,
                                 char **argv UNUSED) {
    printf("Command %s has no run handler!", getCommand());
    errno = ENOSYS;
    return -1;
}
