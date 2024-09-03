#pragma once
#include "common.h"
#include <vector>

void checksumByte(u32& checksum, const u8 currentByte);
u32 calculateChecksum(std::vector<char> input);	// shamelessly stolen from x1nixmzeng
void decompress(u32 srcBuffer, u32 sizeCompressed, u8* dstBuffer);
void sub_3AB6B0();
void sub_3AB500();
void sub_2E8D30();
void sub_3ABA20();
void sub_2E5F60();
