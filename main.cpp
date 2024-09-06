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
            handler.mapInternalPointers();
            handler.mapFileNameStrings();
            handler.mapZPackageFile();
            handler.mapZPackageFilenames();
            handler.mapFileAssets();
            handler.writeListFile();
        }
    }
    else if (args.hasArg("x"))
    {
        // Dump file from Index if -d index
        u32 Index = args.getIntArg("x");
        FileHandler handler(args.getFirstArg());
        if (handler.loadFile())
        {
            handler.decompressChunks();
            handler.mapInternalPointers();
            handler.writeFileFromIndex(Index);
        }
    }
    else if (args.hasArg("t"))
    {
        // test
        FileHandler handler(args.getFirstArg());
        if (handler.loadFile())
        {
            handler.decompressChunks();
            handler.mapInternalPointers();
            handler.mapFileNameStrings();
            handler.mapZPackageFile();
            handler.mapZPackageFilenames();
            handler.mapFileAssets();
            handler.writeListFile();
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