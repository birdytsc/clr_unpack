#pragma once

#include <vector>
#include <string>

#define Naked __declspec( naked )
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define RETAIL
#ifndef RETAIL
#define DEMO
#endif

#pragma pack(push)
#pragma pack(1)

const std::vector<std::string> assetTypes =
{
    "apple",
    "action",
    "animation",
    "model",
    "cutscene",
    "cutscenetest",
    "font",
    "bfdlevel",
    "levelLights",
    "levelZones",
    "mpegspeech",
    "conkernetwork",
    "particle",
    "pixelShader",
    "conkerRes",
    "conkerspeech",
    "conkersurface",
    "task",
    "texPack",
    "text",
    "texture",
    "userinterface",
    "vertexShader",
    "xmldata",
    "package",
    "scenegraph",
    "hits",
    "levelHits"
};

struct FILE_INDEX_ENTRY
{
    u32 Index;          // index into the table
    u32 fileOffset;
    u32 fileSize;
    u8 filePart;
    u8 headerSize;
};

struct RELOCATION_ENTRY
{
    u32 sourceIndex;
    u32 targetIndex;
    u32 relocationCount;
};

#pragma pack(pop)


struct HEADER_TEMPLATE
{
    char name[16];  // < - header start
    u32 unknown_10;
    u32 unknown_14;
    u32 unknown_18;
    u32 unknown_1C; // < - header end
};

struct PACKAGE_HEADER
{
    char name[16];  // < - header start
    u32 unknown_10;
    u32 unknown_14;
    u32 unknown_18;
    u32 unknown_1C; // < - header end
};

struct PACKAGE_ENTRY
{
    u32 fileNameOffset;
    u32 dataOffset;
};

#ifdef RETAIL
struct CAFF_FILE_HEADER // RETAIL
{
    char magic[4];
    char version[16];
    u32 hash;
    u32 numFiles;
    u32 numFilesWithGpu;
    u32 unk_table1;
    u32 unk_table2;
    u32 numRelocs;
    u32 numRelocOffsets;
    u32 unknown_30;
    u32 dataTOCOffset;
    u32 dataOffset;
    u8 unknown_3C; // some kind of endian flag?
    u8 numSections; // count of how many sections (.data .gpu etc.)
    u8 isCompressed; // not set on non-compressed .RBA and set on compressed .RBM
    u8 unknown_3F;
};

struct CAFF_INFO_HEADER // RETAIL
{
    char name[8];
    u32 unknown_08;
    u32 unknown_0C;
    u32 sizeDecompressed;
    u32 unknown_14;
    u32 unknown_18;
    u32 unknown_1C;
    u32 unknown_20;
    u32 sizeCompressed;
};
#endif

#ifdef DEMO
struct CAFF_FILE_HEADER // DEMO
{
    char magic[4];
    char version[16];
    u32 numFiles;
    u32 numFilesWithGpu;
    u32 unk_table1;
    u32 unk_table2;
    u32 numRelocs;
    u32 numRelocOffsets;
    u32 dataTOCOffset;
    u32 dataOffset;
    u8 unknown_34; // some kind of endian flag?
    u8 numSections; // count of how many sections (.data .gpu etc.)
    u8 isCompressed; // not set on non-compressed .RBA and set on compressed .RBM
    u8 unknown_37;
};

struct CAFF_INFO_HEADER // DEMO
{
    char name[8];
    u32 sizeDecompressed;
    u32 unknown_0C;
    u32 unknown_10;
    u32 unknown_14;
    u32 unknown_18;
    u32 unknown_1C;
    u32 sizeCompressed;
};
#endif