# clr_unpack

A Tool to unpack the .RBM CAFF files from the game Conker Live and Reloaded.

can be compiled for both the Retail and Demo versions of the game check common.h and comment out the #define RETAIL line to compile for the demo version.

The compression algorithm is just ripped from the games binary as x86 assembly you can check it out in the caff.cpp file.

I have no idea what algorithm it is however.

Sample usage:

> clr_unpack default.rbm
> 
> clr_unpack_demp default.rbm

What the program does:

> Reads in the .RBM file
> Verifies its Magic and Version
> Decompresses the data chunks inside (usually a .data and .gpu chunk)
> Modifies the header fields that the game uses to check if the file is compressed
> Spits out the file with a .unpacked extension

The unpacked files can be used in-place of the original and even modified as long as the size and offsets remain unchanged.
You will need to completely Re-map the file if you want to change it beyond this, the CAFF files include a Table of Contents
that include the file names and the data offsets for each file in the archive.

All files will have at least a .data offset and optionally a .gpu offset that are matched by an index number.
