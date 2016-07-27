

#ifndef _COMMANDLISTENER_H__
#define _COMMANDLISTENER_H__

#include "FrameworkListener.h"
#include "VoldCommand.h"

class CommandListener : public FrameworkListener {
public:
    CommandListener();
    virtual ~CommandListener() {}

private:
    static void dumpArgs(int argc, char **argv, int argObscure);
    static int sendGenericOkFail(SocketClient *cli, int cond);

    class DumpCmd : public VoldCommand {
    public:
        DumpCmd();
        virtual ~DumpCmd() {}
        int runCommand(SocketClient *c, int argc, char ** argv);
    };

    class VolumeCmd : public VoldCommand {
    public:
        VolumeCmd();
        virtual ~VolumeCmd() {}
        int runCommand(SocketClient *c, int argc, char ** argv);
    };
//
//    class AsecCmd : public VoldCommand {
//    public:
//        AsecCmd();
//        virtual ~AsecCmd() {}
//        int runCommand(SocketClient *c, int argc, char ** argv);
//    private:
//        void listAsecsInDirectory(SocketClient *c, const char *directory);
//    };
//
//    class ObbCmd : public VoldCommand {
//    public:
//        ObbCmd();
//        virtual ~ObbCmd() {}
//        int runCommand(SocketClient *c, int argc, char ** argv);
//    };
//
    class StorageCmd : public VoldCommand {
    public:
        StorageCmd();
        virtual ~StorageCmd() {}
        int runCommand(SocketClient *c, int argc, char ** argv);
    };
//
//    class FstrimCmd : public VoldCommand {
//    public:
//        FstrimCmd();
//        virtual ~FstrimCmd() {}
//        int runCommand(SocketClient *c, int argc, char ** argv);
//    };
};

#endif
