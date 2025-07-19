#include "RpgAssetFile.h"



RpgAssetFileImage::EType RpgAssetFileImage::GetSupportedFileType(const RpgFilePath& filePath) noexcept
{
	const RpgName fileExtension = filePath.GetFileExtension();
	if (fileExtension.GetLength() == 0)
	{
		return MAX_COUNT;
	}

	for (int i = 0; i < MAX_COUNT; ++i)
	{
		if (fileExtension == EXTENSIONS[i])
		{
			return static_cast<EType>(i);
		}
	}

	return MAX_COUNT;
}


RpgAssetFileModel::EType RpgAssetFileModel::GetSupportedFileType(const RpgFilePath& filePath) noexcept
{
	const RpgName fileExtension = filePath.GetFileExtension();
	if (fileExtension.GetLength() == 0)
	{
		return MAX_COUNT;
	}

	for (int i = 0; i < MAX_COUNT; ++i)
	{
		if (fileExtension == EXTENSIONS[i])
		{
			return static_cast<EType>(i);
		}
	}

	return MAX_COUNT;
}
