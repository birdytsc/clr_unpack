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
    compressedBuffers.resize(header.numSections);
    file.seekg(header.dataOffset, std::ios::beg);
    for (size_t i = 0; i < header.numSections; ++i)
    {
        compressedBuffers[i].resize(fileInfos[i].sizeCompressed);
        file.read(compressedBuffers[i].data(), fileInfos[i].sizeCompressed);
    }

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

void FileHandler::decompressChunks()
{

    decompressedBuffers.resize(compressedBuffers.size());
    for (size_t i = 0; i < compressedBuffers.size(); ++i)
    {
        decompressedBuffers[i].resize(fileInfos[i].sizeDecompressed);
        decompress((u32)compressedBuffers[i].data(), fileInfos[i].sizeCompressed, (u8*)decompressedBuffers[i].data());
    }
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
    printf("modified header checksum = %X\n", checksum);
#endif

    // Write header
    outFile.write(headerBuffer.data(), headerBuffer.size());

    // Write decompressed data
    for (size_t i = 0; i < decompressedBuffers.size(); ++i) {
        outFile.write(decompressedBuffers[i].data(), decompressedBuffers[i].size());
    }

    std::cout << "Decompressed file written to: " << outFilename << std::endl;
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
