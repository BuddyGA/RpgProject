#pragma once

#include "core/RpgThreadPool.h"
#include "core/RpgFilePath.h"
#include "render/RpgTexture.h"
#include "RpgAssimpTypes.h"



class RpgAssetTask_ImportTexture : public RpgThreadTask
{
public:
	RpgFilePath SourceFilePath;
	RpgAssimp::FTextureEmbedded SourceEmbedded;
	RpgTextureFormat::EType Format;
	bool bGenerateMipMaps;


public:
	RpgAssetTask_ImportTexture() noexcept;

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
