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

bool FileHandler::mapInternalPointers()
{
    if (header.numSections == 0)
    {
        std::cerr << "Error mapping internal pointers" << std::endl;
        return false;
    }

    u8* data = (u8*)dataBuffers[0].data();
    file_name_buffer_size_ptr = data + header.dataTOCOffset;
    file_name_buffer_offsets_ptr = data + header.dataTOCOffset + 0x4;
    u32* fileNameBufferSize = (u32*)file_name_buffer_size_ptr;
    if (*fileNameBufferSize != 0)
    {
        file_name_buffer_ptr = file_name_buffer_offsets_ptr + (0x4 * header.numFiles);
    }
    else
    {
        file_name_buffer_ptr = file_name_buffer_offsets_ptr;
    }
    file_index_table_ptr = file_name_buffer_ptr + *fileNameBufferSize;
    unused_table_1_ptr = file_index_table_ptr + (sizeof(FILE_INDEX_ENTRY) * header.numFilesWithGpu);    // Another reloc table? have not seen it used yet
    unused_table_2_ptr = unused_table_1_ptr + (12 * header.unk_table1);                                 // <-/
    reloc_table_ptr = unused_table_2_ptr + (4 * header.unk_table2);                                     // Relocation Table
    reloc_offsets_table_ptr = reloc_table_ptr + (12 * header.numRelocs);                                // <-/
    return true;
 }

bool FileHandler::mapFileNameStrings()
{
    u32* bufferSize = (u32*)file_name_buffer_size_ptr;
    if (*bufferSize == 0)
    {
        std::cerr << "could not read filename strings, no strings found." << std::endl;
        return false;
    }

    for (size_t i = 0; i < header.numFiles; i++)
    {
        u32* currentStringOffset = (u32*)file_name_buffer_offsets_ptr + i;
        std::string fileName = (char*)file_name_buffer_ptr + *currentStringOffset;
        listFile[i+1].cFileName = fileName;
    }
    return true;
}

bool FileHandler::mapZPackageFile()
{
    if (header.numRelocs == 0)
    {
        std::cerr << "Error mapping zpackage invalid reloc count" << std::endl;
        return false;
    }

    u8* data = (u8*)dataBuffers[0].data();
    FILE_INDEX_ENTRY* file_index_table = (FILE_INDEX_ENTRY*)file_index_table_ptr;
    u32* reloc_offset = (u32*)reloc_offsets_table_ptr;

    for (size_t i = 0; i < header.numRelocs; i++)
    {
        RELOCATION_ENTRY* currentReloc = (RELOCATION_ENTRY*)reloc_table_ptr + i;

        u32* source_offset = (u32*)file_index_table[currentReloc->sourceIndex - 1].fileOffset;
        u32* target_offset = (u32*)file_index_table[currentReloc->targetIndex - 1].fileOffset;

        u32* source_size = (u32*)file_index_table[currentReloc->sourceIndex - 1].fileSize;
        u32* target_size = (u32*)file_index_table[currentReloc->targetIndex - 1].fileSize;

        for (size_t j = 0; j < currentReloc->relocationCount; j++, reloc_offset++)
        {
            if (*reloc_offset < (u32)source_size)
            {
                u32* address_to_update = (u32*)(data + (u32)source_offset + *reloc_offset);
                *address_to_update += (u32)target_offset;
            }
        }
    }

    return true;
}

bool FileHandler::mapZPackageFilenames()
{
    u8* data = (u8*)dataBuffers[0].data();
    PACKAGE_HEADER* pheader = (PACKAGE_HEADER*)data;
    u8* packageCountPtr = data + sizeof(PACKAGE_HEADER);
    u32* packageCount = (u32*)packageCountPtr;
    u8* packageTablePtr = packageCountPtr + 0x4;
    PACKAGE_ENTRY* packageTable = (PACKAGE_ENTRY*)packageTablePtr;

    FILE_INDEX_ENTRY* file_index_table = (FILE_INDEX_ENTRY*)file_index_table_ptr;

    for (size_t i = 0; i < *packageCount; i++)
    {
        PACKAGE_ENTRY* currentPackageEntry = (PACKAGE_ENTRY*)packageTablePtr + i;
        std::string fileName = (char*)data + currentPackageEntry->fileNameOffset;
        for (size_t j = 0; j < header.numFilesWithGpu; j++)
        {
            if (currentPackageEntry->dataOffset >= file_index_table[j].fileOffset && currentPackageEntry->dataOffset <= (file_index_table[j].fileOffset + file_index_table[j].fileSize) && file_index_table[j].filePart == 1)
            {
                listFile[file_index_table[j].Index].pFileName = fileName;
            }
        }
    }

    return true;
}

bool FileHandler::mapFileAssets()
{
    FILE_INDEX_ENTRY* file_index_table = (FILE_INDEX_ENTRY*)file_index_table_ptr;
    for (size_t i = 0; i < header.numFilesWithGpu; i++)
    {
        if (file_index_table[i].filePart == 1)
        {
            // .data
            u8* assetTypePtr = (u8*)dataBuffers[0].data() + file_index_table[i].fileOffset;
            std::string assetType = (char*)assetTypePtr;
            auto it = std::find(assetTypes.begin(), assetTypes.end(), assetType);
            if (it != assetTypes.end())
            {
                listFile[file_index_table[i].Index].assetType = assetType;
            }
            else
            {
                listFile[file_index_table[i].Index].assetType = "unknown";
            }
            
            listFile[file_index_table[i].Index].dataOffset = file_index_table[i].fileOffset;
            listFile[file_index_table[i].Index].dataSize = file_index_table[i].fileSize;
        }
        else
        {
            // .gpu
            listFile[file_index_table[i].Index].gpuOffset = file_index_table[i].fileOffset;
            listFile[file_index_table[i].Index].gpuSize = file_index_table[i].fileSize;
        }
    }
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

bool FileHandler::writeFileFromIndex(u32 Index)
{
    if (Index > header.numFiles)
    {
        std::cerr << "error writing file, index exceeds file count." << std::endl;
        return false;
    }
    std::vector<std::vector<char>> Buffers;
    FILE_INDEX_ENTRY* file_index_table = (FILE_INDEX_ENTRY*)file_index_table_ptr;

    for (size_t i = 0; i < header.numFiles; i++)
    {
        for (size_t j = 0; j < header.numFilesWithGpu; j++)
        {
            if (file_index_table[j].Index == Index) // Add 1 to match FILE_INDEX_ENTRY indexing
            {
                switch (file_index_table[j].filePart)
                {
                case 1:     // .data
                    Buffers.resize(1);
                    Buffers[0].resize(file_index_table[j].fileSize);
                    memcpy(Buffers[0].data(), dataBuffers[0].data() + file_index_table[j].fileOffset, file_index_table[j].fileSize);
                    break;
                case 2:     // ,gpu
                    Buffers.resize(2);
                    Buffers[1].resize(file_index_table[j].fileSize);
                    memcpy(Buffers[1].data(), dataBuffers[1].data() + file_index_table[j].fileOffset, file_index_table[j].fileSize);
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

bool FileHandler::writeListFile()
{
    std::string outFilename = input_file + ".listfile.csv";
    std::ofstream outFile(outFilename);
    if (!outFile)
    {
        std::cerr << "Error creating listfile: " << outFilename << std::endl;
        return false;
    }

    outFile << "index,asset_type,data_offset,data_size,gpu_offset,gpu_size,c_filename, p_filename" << std::endl;
    for (auto it = listFile.begin(); it != listFile.end(); ++it) {
        outFile << it->first << ","
            << it->second.assetType << ","
            << it->second.dataOffset << ","
            << it->second.dataSize << ","
            << it->second.gpuOffset << ","
            << it->second.gpuSize << ","
            << it->second.cFileName << ","
            << it->second.pFileName << std::endl;
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

void FileHandler::printListFile()
{
    std::cout << header.numFiles << " Files found: " << std::endl;
    for (auto it = listFile.begin(); it != listFile.end(); ++it) {
        std::cout << "index: " << it->first
            << ", type: " << it->second.assetType
            << ", data_offset: " << it->second.dataOffset
            << ", data_size: " << it->second.dataSize
            << ", gpu_offset: " << it->second.gpuOffset
            << ", gpu_size: " << it->second.gpuSize
            << ", cfilename: " << it->second.cFileName
            << ", pfilename: " << it->second.pFileName << std::endl;
    }
}