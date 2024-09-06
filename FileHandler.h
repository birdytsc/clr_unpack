#pragma once
#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <map>

struct LIST_FILE_ENTRY
{
    u32 dataOffset;
    u32 dataSize;
    u32 gpuOffset;
    u32 gpuSize;
    std::string assetType;
    std::string cFileName;
    std::string pFileName;
};

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
    std::vector<std::vector<char>> dataBuffers;
    std::string input_file;
    std::string output_file;
    CAFF_FILE_HEADER header;
    std::vector<CAFF_INFO_HEADER> fileInfos;
    unsigned int fileNameBufferSize;
    std::vector<std::string> fileNameStrings;
    std::vector<FILE_INDEX_ENTRY> fileIndexTable;
    std::map<int, LIST_FILE_ENTRY> listFile;
    // internal pointers
    u8* file_name_buffer_size_ptr;
    u8* file_name_buffer_offsets_ptr;
    u8* file_name_buffer_ptr;
    u8* file_index_table_ptr;
    u8* unused_table_1_ptr;
    u8* unused_table_2_ptr;
    u8* reloc_table_ptr;
    u8* reloc_offsets_table_ptr;

public:
    FileHandler(const std::string& inputFile);
    bool loadFile();
    bool verifyMagic();
    bool verifyVersion();
    bool mapHeaderToStruct();
    bool mapFileInfosToStruct();
    bool mapInternalPointers();
    bool mapFileNameStrings();
    bool mapZPackageFile();
    bool mapZPackageFilenames();
    bool mapFileAssets();
    bool decompressChunks();
    bool writeDecompressedFile();
    bool writeFileFromIndex(u32 Index);
    bool writeListFile();
    void printHeaderInfo();
    void printFileInfos();
    void printListFile();
};