

#ifndef _VOLD_COMMAND_H
#define _VOLD_COMMAND_H

#include "FrameworkCommand.h"

class VoldCommand : public FrameworkCommand {
public:
    VoldCommand(const char *cmd);
    virtual ~VoldCommand() {}
};

#endif
