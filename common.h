#pragma once

#define Naked __declspec( naked )
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define RETAIL
#ifndef RETAIL
#define DEMO
#endif

#ifdef RETAIL
struct CAFF_FILE_HEADER // RETAIL
{
    char magic[4];
    char version[16];
    u32 hash;
    u32 unknown_18;
    u32 unknown_1C;
    u32 unknown_20;
    u32 unknown_24;
    u32 unknown_28;
    u32 unknown_2C;
    u32 unknown_30;
    u32 unknown_34;
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
    u32 unknown_14;
    u32 unknown_18;
    u32 unknown_1C;
    u32 unknown_20;
    u32 unknown_24;
    u32 unknown_28;
    u32 unknown_2C;
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