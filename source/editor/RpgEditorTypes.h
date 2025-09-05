#pragma once

#include "asset/RpgAssetTypes.h"
#include "render/asset/RpgTexture.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogEditor)



struct RpgEditorAssetFile
{
	RpgString Path;
	RpgName Name;
	RpgSharedTexture2D PreviewImage;
	RpgAssetFileType Type{ RpgAssetFileType::NONE };
};



struct RpgEditorAssetFolder
{
	RpgFilePath Path;
	RpgArray<RpgEditorAssetFolder> Subfolders;
	RpgArray<RpgEditorAssetFile> Files;
};
