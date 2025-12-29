#pragma once

#include "gui/widget/RpgGuiWindow.h"
#include "RpgEditorTypes.h"




class RpgEditorAssetBrowserWindow : public RpgGuiWindow
{
public:
	RpgEditorAssetBrowserWindow() noexcept;
	void Refresh() noexcept;


private:
	void ScanAssetFiles(RpgEditorAssetFolder& folder) noexcept;
	void AddWidgetTreeItem(const RpgEditorAssetFolder& folder, float& out_IndentOffset, int& out_FolderIndex, int& out_FileIndex) noexcept;


private:
	RpgEditorAssetFolder AssetFolder;

};
