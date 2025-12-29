#include "RpgEditorAssetBrowser.h"
#include "core/asset/RpgAssetSystem.h"
#include "render/RpgRenderer2D.h"



RpgEditorAssetBrowserWindow::RpgEditorAssetBrowserWindow() noexcept
	: RpgGuiWindow("ed_asset_browser")
{
	ContentChildPadding = RpgRectFloat(4.0f);
	ContentChildSpace = RpgPointFloat(4.0f);
	ContentDirection = RpgGuiLayout::DIRECTION_VERTICAL;
}


void RpgEditorAssetBrowserWindow::Refresh() noexcept
{
	AssetFolder.Path = RpgFileSystem::GetAssetDirPath();
	AssetFolder.Subfolders.Clear();
	AssetFolder.Files.Clear();

	ScanAssetFiles(AssetFolder);

	ClearContentChildren();

	float indentOffset = 0.0f;
	int folderIndex = 0;
	int fileIndex = 0;
	AddWidgetTreeItem(AssetFolder, indentOffset, folderIndex, fileIndex);
	RPG_Check(indentOffset == 0.0f);
}


void RpgEditorAssetBrowserWindow::ScanAssetFiles(RpgEditorAssetFolder& folder) noexcept
{
	const RpgString searchPath = RpgString::Format("%s*", *folder.Path);

	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFileA(*searchPath, &fileData);

	do
	{
		// Ignore '.' and '..'
		if (fileData.cFileName[0] == '.' || (fileData.cFileName[0] == '.' && fileData.cFileName[1] == '.'))
		{
			continue;
		}

		if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			RpgEditorAssetFolder assetSubfolder;
			assetSubfolder.Path = RpgString::Format("%s%s/", *folder.Path, fileData.cFileName);
			ScanAssetFiles(assetSubfolder);

			folder.Subfolders.AddValue(assetSubfolder);
		}
		else
		{
			const RpgFilePath filePath = RpgString::Format("%s%s", *folder.Path, fileData.cFileName);

			RpgAssetInfo info;
			if (g_AssetSystem->IsValidAssetFile(filePath, &info))
			{
				RpgEditorAssetFile& assetFile = folder.Files.Add();
				assetFile.Name = filePath.GetFileName();
				assetFile.Path = info.Path;
				assetFile.Type = info.Type;
			}
		}
	}
	while (FindNextFileA(fileHandle, &fileData));

	FindClose(fileHandle);
}


void RpgEditorAssetBrowserWindow::AddWidgetTreeItem(const RpgEditorAssetFolder& folder, float& out_IndentOffset, int& out_FolderIndex, int& out_FileIndex) noexcept
{
	const RpgName folderName = folder.Path.GetDirectoryName();

	RpgGuiText* widgetFolder = AddContentChild<RpgGuiText>(RpgName::Format("folder_%i", out_FolderIndex++));
	widgetFolder->Position.X += out_IndentOffset;
	widgetFolder->SetTextValue(folderName.ToString());

	out_IndentOffset += 16.0f;

	for (const RpgEditorAssetFolder& folder : folder.Subfolders)
	{
		AddWidgetTreeItem(folder, out_IndentOffset, out_FolderIndex, out_FileIndex);
	}

	for (const RpgEditorAssetFile& file : folder.Files)
	{
		RpgGuiText* widgetFile = AddContentChild<RpgGuiText>(RpgName::Format("file_%i", out_FileIndex++));
		widgetFile->Position.X += out_IndentOffset;
		widgetFile->SetTextValue(file.Name.ToString());
	}

	out_IndentOffset -= 16.0f;
}
