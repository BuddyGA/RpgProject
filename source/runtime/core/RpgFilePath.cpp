#include "RpgFilePath.h"
#include <ShlObj.h>



RpgFilePath RpgFilePath::GetParentDirectoryPath() const noexcept
{
	if (!IsPathValid())
	{
		return RpgFilePath();
	}

	const int lastDirToken = FullPath.FindLastIndexOf('/');

	if (lastDirToken == RPG_INDEX_INVALID)
	{
		return RpgFilePath();
	}

	int secondLastDirToken = RPG_INDEX_INVALID;
	for (int i = lastDirToken - 1; i >= 0 && i != RPG_INDEX_INVALID; --i)
	{
		if (FullPath[i] == '/')
		{
			secondLastDirToken = i;
			break;
		}
	}

	if (secondLastDirToken == RPG_INDEX_INVALID)
	{
		return RpgFilePath();
	}

	return FullPath.Substring(0, secondLastDirToken + 1);
}


RpgString RpgFilePath::GetDirectoryPath() const noexcept
{
	if (!IsPathValid())
	{
		return RpgString();
	}

	const int length = DirectoryPath.GetLength();
	if (length == 0)
	{
		return RpgString();
	}

	char tempDirPath[MAX_PATH];
	RpgPlatformMemory::MemCopy(tempDirPath, DirectoryPath.GetData(), length);
	tempDirPath[length] = '\0';

	return RpgString(tempDirPath);
}


RpgName RpgFilePath::GetDirectoryName() const noexcept
{
	if (!IsPathValid())
	{
		return "";
	}

	const int length = DirectoryName.GetLength();
	if (length == 0)
	{
		return "";
	}

	char tempDirName[RPG_NAME_MAX_COUNT];
	RpgPlatformMemory::MemCopy(tempDirName, DirectoryName.GetData(), length);
	tempDirName[length] = '\0';

	return tempDirName;
}


RpgName RpgFilePath::GetFileName() const noexcept
{
	if (!IsPathValid())
	{
		return "";
	}

	const int length = FileName.GetLength();
	if (length == 0)
	{
		return "";
	}

	char tempFileName[RPG_NAME_MAX_COUNT];
	RpgPlatformMemory::MemCopy(tempFileName, FileName.GetData(), length);
	tempFileName[length] = '\0';

	return tempFileName;
}


RpgName RpgFilePath::GetFileExtension() const noexcept
{
	if (!IsPathValid())
	{
		return "";
	}

	const int length = FileExt.GetLength();
	if (length == 0)
	{
		return "";
	}

	char tempFileExt[RPG_NAME_MAX_COUNT];
	RpgPlatformMemory::MemCopy(tempFileExt, FileExt.GetData(), length);
	tempFileExt[length] = '\0';

	return tempFileExt;
}


void RpgFilePath::InitializeInternal()
{
	DirectoryPath = RpgStringView();
	DirectoryName = RpgStringView();
	FileName = RpgStringView();
	FileExt = RpgStringView();

	if (FullPath.IsEmpty())
	{
		return;
	}

	const int length = FullPath.GetLength();
	const int dirToken = FullPath.FindLastIndexOf('/');
	const int extToken = FullPath.FindLastIndexOf('.');

	// Directory path
	if (dirToken != RPG_INDEX_INVALID)
	{
		DirectoryPath = FullPath.SubstringView(0, dirToken + 1);
	}

	// Directory name
	if (DirectoryPath.GetLength() > 0)
	{
		int prevDirToken = RPG_INDEX_INVALID;
		for (int i = dirToken - 1; i >= 0 && i != RPG_INDEX_INVALID; --i)
		{
			if (FullPath[i] == '/')
			{
				prevDirToken = i;
				break;
			}
		}

		if (prevDirToken != RPG_INDEX_INVALID)
		{
			const int dirNameLength = dirToken - (prevDirToken + 1);
			DirectoryName = FullPath.SubstringView(prevDirToken + 1, dirNameLength);
		}
		else
		{
			DirectoryName = FullPath.SubstringView(0, length - 1);
		}
	}


	// if last character is '/' ignore FileName and FileExt
	if (dirToken == length - 1)
	{
		return;
	}

	if (DirectoryPath.GetLength() > 0)
	{
		FileName = (extToken == RPG_INDEX_INVALID) ? FullPath.SubstringView(dirToken + 1, length - dirToken - 1) : FullPath.SubstringView(dirToken + 1, extToken - dirToken - 1);
	}
	else
	{
		FileName = (extToken == RPG_INDEX_INVALID) ? FullPath.SubstringView(0, length) : FullPath.SubstringView(0, extToken);
	}

	if (FileName.GetLength() > 0 && extToken != RPG_INDEX_INVALID)
	{
		FileExt = FullPath.SubstringView(extToken, length - extToken);
	}
}


bool RpgFilePath::IsPathValid() const noexcept
{
	const int length = FullPath.GetLength();
	if (length == 0)
	{
		return false;
	}

	if (length == 1 && FullPath[0] == '.')
	{
		return false;
	}

	if (length == 2 && (FullPath[0] == '.' || FullPath[1] == '.'))
	{
		return false;
	}

	return true;
}



namespace RpgFileSystem
{
	static RpgFilePath ExecutablePath;
	static RpgString UserAppDataLocalDirPath;
	static RpgString UserTempDirPath;
	static RpgString ProjectDirPath;
	static RpgString SourceDirPath;
	static RpgString AssetDirPath;
	static RpgString AssetRawDirPath;
};


void RpgFileSystem::Initialize() noexcept
{
	char tempDirPath[MAX_PATH];
	RpgString tempPathString;

	// Executable
	{
		char executablePath[MAX_PATH];
		GetModuleFileNameA(NULL, executablePath, MAX_PATH);

		tempPathString = executablePath;
		tempPathString.ReplaceInPlace('\\', '/');
		ExecutablePath = tempPathString;
	}

	// User/AppData/Local
	{
		RpgPlatformMemory::MemZero(tempDirPath, MAX_PATH);

		PWSTR wPath;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &wPath);
		const int n = static_cast<int>(wcstombs(tempDirPath, wPath, MAX_PATH));
		tempDirPath[n] = '/';
		tempDirPath[n + 1] = '\0';
		CoTaskMemFree(wPath);
		tempPathString = tempDirPath;
		tempPathString.ReplaceInPlace('\\', '/');
		UserAppDataLocalDirPath = tempPathString;
	}

	// Temp 
	{
		GetTempPathA(MAX_PATH, tempDirPath);
		tempPathString = tempDirPath;
		tempPathString.ReplaceInPlace('\\', '/');
		UserTempDirPath = tempPathString;
	}

#ifdef RPG_BUILD_SHIPPING
	ProjectDirPath = ExecutablePath.GetDirectoryPath();
#else
	ProjectDirPath = ExecutablePath.GetParentDirectoryPath().GetParentDirectoryPath().ToString();
#endif // RPG_BUILD_SHIPPING

	SourceDirPath = ProjectDirPath + "source/";
	AssetDirPath = ProjectDirPath + "assets/";
	AssetRawDirPath = ProjectDirPath + "assets/raw/";
}


RpgString RpgFileSystem::GetExecutableDirPath() noexcept
{
	return ExecutablePath.GetDirectoryPath();
}


const RpgString& RpgFileSystem::GetUserAppDataLocalDirPath() noexcept
{
	return UserAppDataLocalDirPath;
}


const RpgString& RpgFileSystem::GetUserTempDirPath() noexcept
{
	return UserTempDirPath;
}


const RpgString& RpgFileSystem::GetProjectDirPath() noexcept
{
	return ProjectDirPath;
}


const RpgString& RpgFileSystem::GetSourceDirPath() noexcept
{
	return SourceDirPath;
}


const RpgString& RpgFileSystem::GetAssetDirPath() noexcept
{
	return AssetDirPath;
}


const RpgString& RpgFileSystem::GetAssetRawDirPath() noexcept
{
	return AssetRawDirPath;
}


void RpgFileSystem::IterateFolders(RpgArray<RpgFilePath>& out_FolderPaths, const RpgString& folderPath, bool bIncludeSubfolder) noexcept
{
	const RpgString searchPath = RpgString::Format("%s*", *folderPath);

	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFileA(*searchPath, &fileData);

	do
	{
		// Ignore '.' and '..'
		if (fileData.cFileName[0] == '.' || (fileData.cFileName[0] == '.' && fileData.cFileName[1] == '.'))
		{
			continue;
		}

		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			const RpgString subfolderPath = RpgString::Format("%s%s/", *folderPath, fileData.cFileName);
			//RPG_Log(RpgLogSystem, "%s", *subfolderPath);
			out_FolderPaths.AddValue(subfolderPath);

			IterateFolders(out_FolderPaths, subfolderPath, bIncludeSubfolder);
		}
	}
	while (FindNextFileA(fileHandle, &fileData));

	FindClose(fileHandle);
}


void RpgFileSystem::IterateFiles(RpgArray<RpgFilePath>& out_FilePaths, const RpgString& folderPath, bool bIncludeSubfolder, const char* filterExt) noexcept
{
	const RpgString searchPath = RpgString::Format("%s*", *folderPath);

	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFileA(*searchPath, &fileData);

	do
	{
		// Ignore '.' and '..'
		if (fileData.cFileName[0] == '.' || (fileData.cFileName[0] == '.' && fileData.cFileName[1] == '.'))
		{
			continue;
		}

		if (bIncludeSubfolder && (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			const RpgString subfolderPath = RpgString::Format("%s%s/", *folderPath, fileData.cFileName);
			//RPG_Log(RpgLogSystem, "%s", *subfolderPath);

			IterateFiles(out_FilePaths, subfolderPath, bIncludeSubfolder, filterExt);
		}
		else
		{
			const RpgFilePath filePath = RpgString::Format("%s%s", *folderPath, fileData.cFileName);
			
			if (filterExt == nullptr || filterExt == "" || filePath.GetFileExtension() == filterExt)
			{
				//RPG_Log(RpgLogSystem, "%s", *filePath);
				out_FilePaths.AddValue(filePath);
			}
		}
	}
	while (FindNextFileA(fileHandle, &fileData));

	FindClose(fileHandle);
}


bool RpgFileSystem::ReadFromFile(const RpgString& filePath, RpgArray<uint8_t>& out_Data) noexcept
{
	out_Data.Clear();

	HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_READ);
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	const size_t fileSizeBytes = RpgPlatformFile::FileGetSize(fileHandle);
	RPG_Check(fileSizeBytes < RPG_MAX_COUNT);
	out_Data.Resize(static_cast<int>(fileSizeBytes));

	const bool bReadSuccess = RpgPlatformFile::FileRead(fileHandle, out_Data.GetData(), fileSizeBytes);
	RpgPlatformFile::FileClose(fileHandle);

	return bReadSuccess;
}


bool RpgFileSystem::WriteToFile(const RpgString& filePath, const void* data, size_t sizeBytes) noexcept
{
	HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	const bool bWriteSuccess = RpgPlatformFile::FileWrite(fileHandle, data, sizeBytes);
	RpgPlatformFile::FileClose(fileHandle);

	return bWriteSuccess;
}
