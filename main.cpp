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
            handler.mapFileIndexTable();
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
            handler.writeFileFromIndex(Index, false);
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
            handler.mapInternalPointers();
            handler.mapFileIndexTable();
            handler.mapZPackageFile();
            if (Index)
            {
                handler.writeFileFromIndex(Index, true);
            }
            else
            {
                handler.writeDecompressedFile(true);
            }
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
            handler.mapFileIndexTable();
            handler.mapZPackageFile();
            handler.writeDecompressedFile(true);
            handler.writeFileFromIndex(1, true);
            handler.writeFileFromIndex(2, true);
            handler.writeFileFromIndex(3, true);
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
            handler.writeDecompressedFile(false);
        }
    }

    return 0;
}