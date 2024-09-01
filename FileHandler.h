#pragma once
#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <iomanip>

class FileHandler
{
private:
    const char* expectedMagic = "CAFF"; //  expected magic bytes
#ifdef RETAIL
    size_t headerSize = 384; // RETAIL header size
    const char* expectedVersion = "28.01.05.0031";  // expected version string
#else
    size_t headerSize = 344; // DEMO header size
    const char* expectedVersion = "24.09.03.0026";  // expected version string
#endif
    std::vector<char> headerBuffer;
    std::vector<std::vector<char>> compressedBuffers;
    std::vector<std::vector<char>> decompressedBuffers;
    std::string input_file;
    std::string output_file;
    CAFF_FILE_HEADER header;
    std::vector<CAFF_INFO_HEADER> fileInfos;

public:
    FileHandler(const std::string& inputFile);
    bool loadFile();
    bool verifyMagic();
    bool verifyVersion();
    bool mapHeaderToStruct();
    bool mapFileInfosToStruct();
    bool decompressChunks();
    bool writeDecompressedFile();
    void printHeaderInfo();
    void printFileInfos();
};