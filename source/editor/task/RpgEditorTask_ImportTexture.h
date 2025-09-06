#pragma once

#include "core/RpgThreadPool.h"
#include "render/asset/RpgTexture.h"
#include "RpgAssimpTypes.h"
#include "../RpgEditorTypes.h"



class RpgEditorTask_ImportTexture : public RpgThreadTask
{
public:
	RpgFilePath SourceFilePath;
	RpgAssimp::FTextureEmbedded SourceEmbedded;
	RpgTextureFormat::EType Format;
	bool bGenerateMipMaps;


public:
	RpgEditorTask_ImportTexture() noexcept;

	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;

	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgAsyncTask_ImportTexture";
	}


	[[nodiscard]] inline RpgSharedTexture2D GetResult() noexcept
	{
		return std::move(Result);
	}


private:
	RpgSharedTexture2D Result;

};
