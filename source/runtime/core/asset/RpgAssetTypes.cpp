#include "RpgAssetTypes.h"
#include "../RpgConsoleSystem.h"
#include "thirdparty/xxhash/xxhash.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogAsset, VERBOSITY_DEBUG)


RpgFilePath RpgAssetStream::Write(RpgAssetObject* asset, const char* dstFolder) noexcept
{
	RPG_Check(asset);
	RPG_Check(dstFolder);

	// create folder
	const RpgString folderPath = RpgString::Format("%s%s/", *RpgFileSystem::GetAssetDirPath(), dstFolder);
	RpgFileSystem::CreateFolder(folderPath);

	// output file path
	const RpgFilePath filePath = RpgString::Format("%s%s.rpga", *folderPath, *asset->GetAssetName());

	// open file
	HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
	RPG_ValidateV(fileHandle && fileHandle != INVALID_HANDLE_VALUE, "Open file failed! (FilePath: %s)", *filePath);

	// data asset
	RpgBinaryStreamWriter dataAssetWriter;
	{
		// begin data asset
		dataAssetWriter.Write(RPG_ASSET_FILE_ASSET_DATA);

		// data asset
		asset->AssetStreamWrite(dataAssetWriter);

		// end data asset
		dataAssetWriter.Write(RPG_ASSET_FILE_ASSET_DATA);

		RPG_Check(dataAssetWriter.GetByteArraySize() < UINT32_MAX);
	}

	// header
	RpgAssetFileHeader header;
	{
		header.Magix = RPG_ASSET_FILE_HEADER;
		header.Type = static_cast<uint16_t>(asset->GetAssetFileType());
		header.Version = asset->GetAssetFileVersion();
		header.AssetReferenceCount = 0;
		header.AssetReferenceSizeBytes = 0;
		header.AssetDataSizeBytes = static_cast<uint32_t>(dataAssetWriter.GetByteArraySize());
		header.AssetClassName = asset->GetAssetClassName();
	}

	// write to file (header)
	RpgPlatformFile::FileWrite(fileHandle, &header, sizeof(RpgAssetFileHeader));

	// write to file (data)
	RpgPlatformFile::FileWrite(fileHandle, dataAssetWriter.GetByteArrayData(), dataAssetWriter.GetByteArraySize());

	// write to file (end-of-file)
	const uint32_t eof = RPG_ASSET_FILE_EOF;
	RpgPlatformFile::FileWrite(fileHandle, &eof, sizeof(uint32_t));

	// close
	RpgPlatformFile::FileClose(fileHandle);

	return filePath;
}


void RpgAssetStream::Read(RpgAssetObject* asset) noexcept
{
	RPG_Check(asset);

	const RpgString& path = asset->GetAssetPath();
	RPG_Check(!path.IsEmpty());

	// load from file
	const RpgFilePath filePath = RpgString::Format("%s%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

	RpgArray<uint8_t> data;
	RPG_ValidateV(RpgFileSystem::ReadFromFile(filePath.ToString(), data), "Read file data failed! (FilePath: %s)", *filePath);

	RpgBinaryStreamReader reader(data);

	// header
	RpgAssetFileHeader header;
	{
		reader.Read(header);
		RPG_Check(header.Magix == RPG_ASSET_FILE_HEADER);
		RPG_Check(header.Type == static_cast<uint16_t>(asset->GetAssetFileType()));
	}

	// data
	{
		uint32_t begin = 0;
		reader.Read(begin);
		RPG_Check(begin == RPG_ASSET_FILE_ASSET_DATA);

		asset->AssetStreamRead(reader, header.Version);

		uint32_t end = 0;
		reader.Read(end);
		RPG_Check(end == RPG_ASSET_FILE_ASSET_DATA);

		RPG_Check(header.AssetDataSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader));
	}

	// end-of-file
	uint32_t eof = 0;
	reader.Read(eof);
	RPG_Check(eof == RPG_ASSET_FILE_EOF);
}
