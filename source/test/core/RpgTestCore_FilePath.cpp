#include "RpgTestCore.h"
#include "core/RpgFilePath.h"



void RpgTest::Core::Test_FilePath() noexcept
{
	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true);

	for (int i = 0; i < filePaths.GetCount(); ++i)
	{
		RPG_Assert(filePaths[i].IsFilePath());
	}

	RpgArray<RpgFilePath> folderPaths;
	RpgFileSystem::IterateFolders(folderPaths, RpgFileSystem::GetAssetDirPath(), true);

	for (int i = 0; i < folderPaths.GetCount(); ++i)
	{
		RPG_Assert(folderPaths[i].IsDirectoryPath());
	}
}
