#include "FileHandler.h"
#include "caff.h"


FileHandler::FileHandler(const std::string& inputFile)
{
    input_file = inputFile;
    headerBuffer.resize(headerSize);
};

bool FileHandler::loadFile()
{
    std::ifstream file(input_file, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file: " << input_file << std::endl;
        return false;
    }

    // Read header
    file.read(headerBuffer.data(), headerBuffer.size());
    if (file.gcount() != headerBuffer.size())
    {
        std::cerr << "Error reading header: Unexpected file size" << std::endl;
        return false;
    }

    // Verify magic
    if (!verifyMagic())
    {
        std::cerr << "Invalid magic number" << std::endl;
        return false;
    }

    // Verify version
    if (!verifyVersion())
    {
        std::cerr << "Version verification failed" << std::endl;
        return false;
    }

    // Map header to struct
    if (!mapHeaderToStruct())
    {
        std::cerr << "Failed to map header to struct" << std::endl;
        return false;
    }

    // Map fileinfos to struct
    if (!mapFileInfosToStruct())
    {
        std::cerr << "Failed to map FileInfos to struct" << std::endl;
        return false;
    }

    // Read data into buffers
    dataBuffers.resize(header.numSections);
    file.seekg(header.dataOffset, std::ios::beg);
    for (size_t i = 0; i < header.numSections; ++i)
    {
        dataBuffers[i].resize(fileInfos[i].sizeCompressed);
        file.read(dataBuffers[i].data(), fileInfos[i].sizeCompressed);
    }

    file.close();
    return true;
}

bool FileHandler::verifyMagic()
{
    return std::strncmp(headerBuffer.data(), expectedMagic, 4) == 0;
}

bool FileHandler::verifyVersion()
{
    return std::strncmp(headerBuffer.data() + 4, expectedVersion, 16) == 0;
}

bool FileHandler::mapHeaderToStruct()
{
    std::memcpy(&header, headerBuffer.data(), sizeof(CAFF_FILE_HEADER));
    return true;
}

bool FileHandler::mapFileInfosToStruct()
{
    size_t fileInfoOffset = sizeof(CAFF_FILE_HEADER);
    size_t fileInfoSize = sizeof(CAFF_INFO_HEADER) * header.numSections;
    if (fileInfoOffset + fileInfoSize > headerBuffer.size()) {
        std::cerr << "Error: Header buffer too small for FileInfo structs" << std::endl;
        return false;
    }
    fileInfos.resize(header.numSections);
    std::memcpy(fileInfos.data(), headerBuffer.data() + fileInfoOffset, fileInfoSize);
    return true;
}

bool FileHandler::mapFileNameStrings()
{
    if (header.numSections == 0)
    {
        std::cerr << "Error reading filename strings: invalid section count" << std::endl;
        return false;
    }

    // Read bufferSize
    std::memcpy(&fileNameBufferSize, dataBuffers[0].data() + header.dataIndexTableOffset, sizeof(unsigned int));
    if (fileNameBufferSize == 0)
    {
        std::cerr << "filename string table was empty" << std::endl;  // retail seems to strip the symbols so add empty ones
        fileNameStrings.resize(header.numFiles);
        return false;
    }

    // Calculate the start of the string buffer
    unsigned int stringBufferStart = header.dataIndexTableOffset + header.numFiles * sizeof(unsigned int) + 0x4;

    // Vector to store the file offsets
    std::vector<unsigned int> fileOffsets(header.numFiles);
    std::memcpy(fileOffsets.data(), dataBuffers[0].data() + header.dataIndexTableOffset + sizeof(unsigned int), header.numFiles * sizeof(unsigned int));

    // Parse strings
    for (unsigned int i = 0; i < header.numFiles; ++i) {
        unsigned int stringOffset = stringBufferStart + fileOffsets[i];

        // Ensure we don't read past the buffer
        if (stringOffset >= dataBuffers[0].size()) {
            std::cerr << "Error: String offset out of bounds for index " << i << std::endl;
            fileNameStrings.push_back(""); // Add an empty string for invalid offsets
            continue;
        }

        // Find the null terminator
        const char* stringStart = reinterpret_cast<const char*>(dataBuffers[0].data() + stringOffset);
        size_t stringLength = strnlen(stringStart, fileNameBufferSize - fileOffsets[i]);

        // Add the string to our vector
        fileNameStrings.emplace_back(stringStart, stringLength);
    }
}

bool FileHandler::mapFileIndexTable()
{
    unsigned int fileIndexTableStart;
    if (fileNameBufferSize == 0)
    {
        fileIndexTableStart = header.dataIndexTableOffset + 0x4;
    }
    else
    {
        fileIndexTableStart = header.dataIndexTableOffset + 0x4 + (header.numFiles * 0x4) + fileNameBufferSize;
    }

    fileIndexTable.resize(header.numFilesWithGpu);
    std::memcpy(fileIndexTable.data(), dataBuffers[0].data() + fileIndexTableStart, sizeof(FILE_INDEX_ENTRY) * header.numFilesWithGpu);

    return true;
}

bool FileHandler::decompressChunks()
{
    if(header.isCompressed == 0)
    {
        std::cerr << "Error file is already decompressed:" << std::endl;
        return false;
    }
    std::vector<std::vector<char>> decompressedBuffers;
    decompressedBuffers.resize(dataBuffers.size());
    for (size_t i = 0; i < dataBuffers.size(); ++i)
    {
        decompressedBuffers[i].resize(fileInfos[i].sizeDecompressed);
        decompress((u32)dataBuffers[i].data(), fileInfos[i].sizeCompressed, (u8*)decompressedBuffers[i].data());
        dataBuffers[i].resize(fileInfos[i].sizeDecompressed);
        dataBuffers[i] = decompressedBuffers[i];
    }
    return true;
}

bool FileHandler::writeDecompressedFile()
{
    std::string outFilename = input_file + ".unpacked";
    std::ofstream outFile(outFilename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error creating output file: " << outFilename << std::endl;
        return false;
    }

    // Modify header

    header.isCompressed = 0;
    std::memcpy(headerBuffer.data(), &header, sizeof(CAFF_FILE_HEADER));

    // Modify InfoSections

    for (size_t i = 0; i < fileInfos.size(); i++)
    {
        fileInfos[i].sizeCompressed = fileInfos[i].sizeDecompressed;
    }

    std::memcpy(headerBuffer.data() + sizeof(CAFF_FILE_HEADER), fileInfos.data(), sizeof(CAFF_INFO_HEADER) * fileInfos.size());

    // if RETAIL modify the header checksum - not needed on the DEMO 
#ifdef RETAIL
    u32 checksum = calculateChecksum(headerBuffer);
    std::memcpy(headerBuffer.data() + 0x14, &checksum, sizeof(checksum));
    std::cout << "modified header checksum = " << std::hex << checksum << std::endl;
#endif

    // Write header
    outFile.write(headerBuffer.data(), headerBuffer.size());

    // Write decompressed data
    for (size_t i = 0; i < dataBuffers.size(); ++i)
    {
        outFile.write(dataBuffers[i].data(), dataBuffers[i].size());
    }

    std::cout << "Decompressed file written to: " << std::hex << outFilename << std::endl;
    outFile.close();
    return true;
}

bool FileHandler::writeListFile()
{
    std::string outFilename = input_file + ".listfile.txt";
    std::ofstream outFile(outFilename);
    if (!outFile)
    {
        std::cerr << "Error creating listfile: " << outFilename << std::endl;
        return false;
    }

    outFile << "Files found: " << header.numFiles << std::endl;
    outFile << std::left << std::setw(8) << "Index" << std::setw(16) << "Type" << "Filename" << std::endl << std::endl;
    for (size_t i = 0; i < header.numFiles; i++)
    {
        std::string assetType;
        for (size_t j = 0; j < header.numFilesWithGpu; ++j)
        {
            if (fileIndexTable[j].Index == (i + 1)) // Add 1 to match FILE_INDEX_ENTRY indexing
            {
                if (fileIndexTable[j].filePart == 1)
                {
                    assetType = dataBuffers[0].data() + fileIndexTable[j].fileOffset;
                }
            }
        }
        if (fileNameStrings[i].empty())
            fileNameStrings[i] = "unknown";
        outFile << std::left << std::setw(8) << i+1 << std::setw(16) << assetType << fileNameStrings[i] << std::endl;
    }

    outFile.close();
    return true;
}

bool FileHandler::writeFileFromIndex(u32 Index)
{
    std::vector<std::vector<char>> Buffers;
    for (size_t i = 0; i < header.numFiles; i++)
    {
        for (size_t j = 0; j < header.numFilesWithGpu; j++)
        {
            if (fileIndexTable[j].Index == Index) // Add 1 to match FILE_INDEX_ENTRY indexing
            {
                switch (fileIndexTable[j].filePart)
                {
                case 1:     // .data
                    Buffers.resize(1);
                    Buffers[0].resize(fileIndexTable[j].fileSize);
                    memcpy(Buffers[0].data(), dataBuffers[0].data() + fileIndexTable[j].fileOffset, fileIndexTable[j].fileSize);
                    break;
                case 2:     // ,gpu
                    Buffers.resize(2);
                    Buffers[1].resize(fileIndexTable[j].fileSize);
                    memcpy(Buffers[1].data(), dataBuffers[1].data() + fileIndexTable[j].fileOffset, fileIndexTable[j].fileSize);
                    break;
                }
            }
        }
    }

    std::ostringstream s;
    std::string outFilename;
    std::ofstream outFile;

    for (int i = 0; i < Buffers.size(); i++)
    {
        switch (i)
        {
        case 0:
            s.str("");
            s << Index << ".data";
            outFilename = s.str();
            outFile.open(outFilename, std::ios::binary);
            if (!outFile)
            {
                std::cerr << "error writing .data from file" << std::endl;
                return false;
            }
            outFile.write(Buffers[i].data(), Buffers[i].size());
            outFile.close();
            break;
        case 1:
            s.str("");
            s << Index << ".gpu";
            outFilename = s.str();
            outFile.open(outFilename, std::ios::binary);
            if (!outFile)
            {
                std::cerr << "error writing .gpu from file" << std::endl;
                return false;
            }
            outFile.write(Buffers[i].data(), Buffers[i].size());
            outFile.close();
            break;
        }
    }
    return true;
}

void FileHandler::printHeaderInfo()
{
    std::cout << "Magic: " << std::string(header.magic, 4) << std::endl;
    std::cout << "Version: " << header.version << std::endl;
    std::cout << "Number of Sections: " << (u32)header.numSections << std::endl;
}

void FileHandler::printFileInfos()
{
    for (size_t i = 0; i < fileInfos.size(); ++i) {
        std::cout << "Section " << i + 1 << ":" << std::endl;
        std::cout << "  Name: " << fileInfos[i].name << std::endl;
        std::cout << "  SizeCompressed: " << fileInfos[i].sizeCompressed << std::endl;
        std::cout << "  SizeDecompressed: " << fileInfos[i].sizeDecompressed << std::endl;
    }
}

void FileHandler::printFileNames()
{
    std::cout << header.numFiles << " Files found: " << std::endl;
    for (size_t i = 0; i < fileNameStrings.size(); ++i) {
        std::cout << "  Index " << i + 1 << ": " << fileNameStrings[i] << std::endl;
    }
}