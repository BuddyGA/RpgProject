#pragma once

#include "asset/RpgAssetTypes.h"
#include "render/RpgTexture.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogEditor)



struct RpgEditorAssetInfo
{
	RpgFilePath Path;
	RpgSharedTexture2D Icon;
	RpgAssetFileType Type{ RpgAssetFileType::NONE };
};
