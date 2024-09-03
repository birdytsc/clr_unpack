#include "main.h"
#include <iostream>

int main(int argc, char* argv[])
{
    CommandLineArgs args(argc, argv);
    // args.printArgs();

    if (args.hasArg("l"))
    {
        // Generate listfile if -l
        FileHandler handler(args.getFirstArg());
        if (handler.loadFile())
        {
            handler.decompressChunks();
            handler.mapFileNameStrings();
            handler.mapFileIndexTable();
            handler.writeListFile();
        }
    }
    else if (args.hasArg("d"))
    {
        // Dump file from Index if -d index
        u32 Index = args.getIntArg("d");
        FileHandler handler(args.getFirstArg());
        if (handler.loadFile())
        {
            handler.decompressChunks();
            handler.mapFileNameStrings();
            handler.mapFileIndexTable();
            handler.writeFileFromIndex(Index);
        }
    }
    else
    {
        // unpack the file if no other args
        FileHandler handler(args.getFirstArg());
        if (handler.loadFile())
        {
            handler.decompressChunks();
            handler.printHeaderInfo();
            handler.printFileInfos();
            handler.writeDecompressedFile();
        }
    }

    return 0;
}