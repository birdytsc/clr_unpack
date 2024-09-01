#include "main.h"

int main(int argc, char* argv[])
{
    CommandLineArgs args(argc, argv);
    args.printArgs();

    FileHandler handler(args.getFirstArg());

    if (handler.loadFile())
    {
        if(handler.decompressChunks())
            handler.writeDecompressedFile();
        handler.printHeaderInfo();
        handler.printFileInfos();
    }

    return 0;
}