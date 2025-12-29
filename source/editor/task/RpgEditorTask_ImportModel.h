#pragma once

#include "core/RpgThreadPool.h"
#include "animation/RpgAnimationTypes.h"
#include "RpgAssimpTypes.h"
#include "../RpgEditorTypes.h"
#include "../RpgEditorModel.h"




class RpgEditorTask_ImportModel : public RpgThreadTask
{
public:
	RpgFilePath SourceFilePath;
	float Scale;
	bool bImportMaterialTexture;
	bool bImportSkeleton;
	bool bImportAnimation;
	bool bGenerateTextureMipMaps;
	bool bIgnoreTextureNormals;


public:
	RpgEditorTask_ImportModel() noexcept;

	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;

	virtual const char* GetTaskName() const noexcept
	{
		return "RpgAsyncTask_ImportModel";
	}


	[[nodiscard]] inline RpgArray<RpgSharedEditorModel> GetImportedModels() noexcept
	{
		return std::move(ImportedModels);
	}

	[[nodiscard]] inline RpgSharedAnimationSkeleton GetImportedSkeleton() noexcept
	{
		return std::move(ImportedSkeleton);
	}

	[[nodiscard]] inline RpgArray<RpgSharedAnimationClip> GetImportedAnimations() noexcept
	{
		return std::move(ImportedAnimations);
	}


private:
	void ExtractMaterialTextures(const aiScene* assimpScene);
	void ExtractSkeleton(const aiMesh* assimpMesh) noexcept;
	void ExtractMeshesFromNode(const aiScene* assimpScene, const aiNode* assimpNode) noexcept;
	void ExtractAnimations(const aiScene* assimpScene) noexcept;


private:
	RpgArray<class RpgEditorTask_ImportTexture*> ImportTextureTasks;
	RpgArray<RpgAssimp::FMaterialPhong> IntermediateMaterialPhongs;
	RpgArray<RpgAssimp::FModel> IntermediateModels;

	RpgArray<RpgSharedEditorModel> ImportedModels;
	RpgArray<RpgSharedAnimationClip> ImportedAnimations;
	RpgSharedAnimationSkeleton ImportedSkeleton;

};
