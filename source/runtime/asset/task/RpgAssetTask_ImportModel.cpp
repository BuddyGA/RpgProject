#include "RpgAssetTask_ImportModel.h"
#include "RpgAssetTask_ImportTexture.h"
#include "../RpgAssetImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


static_assert(sizeof(DirectX::XMFLOAT3) == sizeof(aiVector3D), "Size of float3 is not equals!");
static_assert(sizeof(DirectX::XMFLOAT3) == sizeof(aiVector3D), "Size of float3 is not equals!");
static_assert(sizeof(DirectX::XMFLOAT2) == sizeof(aiVector2D), "Size of float2 is not equals!");
static_assert(sizeof(RpgMatrixTransform) == sizeof(aiMatrix4x4), "Size of matrix is not equals!");



namespace RpgAssimp
{
	[[nodiscard]] inline static RpgVector3 ToVector3(const aiVector3D& assimpVector) noexcept
	{
		return RpgVector3(assimpVector.x, assimpVector.y, assimpVector.z);
	}


	[[nodiscard]] inline static RpgQuaternion ToQuaternion(const aiQuaternion& assimpQuat) noexcept
	{
		return RpgQuaternion(assimpQuat.x, assimpQuat.y, assimpQuat.z, assimpQuat.w);
	}


	[[nodiscard]] inline static RpgMatrixTransform ToMatrixTransform(const aiMatrix4x4& assimpMatrix) noexcept
	{
		RpgMatrixTransform matrix;
		RpgPlatformMemory::MemCopy(&matrix, &assimpMatrix, sizeof(RpgMatrixTransform));
		matrix.TransposeInPlace();

		return matrix;
	}


	[[nodiscard]] inline static RpgTextureFormat::EType ToTextureFormat(aiTextureType assimpTextureType) noexcept
	{
		switch (assimpTextureType)
		{
			case aiTextureType_DIFFUSE: return RpgTextureFormat::TEX_2D_BC3U;
			case aiTextureType_SPECULAR: return RpgTextureFormat::TEX_2D_BC4U;
			case aiTextureType_NORMALS: return RpgTextureFormat::TEX_2D_BC5S;

			/*
			case aiTextureType_AMBIENT:
			case aiTextureType_EMISSIVE:
			case aiTextureType_HEIGHT:
			case aiTextureType_SHININESS:
			case aiTextureType_OPACITY:
			case aiTextureType_DISPLACEMENT:
			case aiTextureType_LIGHTMAP:
			case aiTextureType_REFLECTION:
			case aiTextureType_BASE_COLOR:
			case aiTextureType_NORMAL_CAMERA:
			case aiTextureType_EMISSION_COLOR:
			case aiTextureType_METALNESS:
			case aiTextureType_DIFFUSE_ROUGHNESS:
			case aiTextureType_AMBIENT_OCCLUSION:
			case aiTextureType_SHEEN:
			case aiTextureType_CLEARCOAT:
			case aiTextureType_TRANSMISSION:
			*/

			default: break;
		}

		RPG_Check(0);
		return RpgTextureFormat::NONE;
	}

};



RpgAssetTask_ImportModel::RpgAssetTask_ImportModel() noexcept
{
	Scale = 1.0f;
	bImportMaterialTexture = false;
	bImportSkeleton = false;
	bImportAnimation = false;
	bGenerateTextureMipMaps = false;
}


void RpgAssetTask_ImportModel::Reset() noexcept
{
	RpgThreadTask::Reset();

	SourceFilePath.Clear(true);
	Scale = 1.0f;
	bImportMaterialTexture = false;
	bImportSkeleton = false;
	bImportAnimation = false;
	bGenerateTextureMipMaps = false;
	bIgnoreTextureNormals = false;
	IntermediateMaterialPhongs.Clear(true);
	IntermediateModels.Clear(true);
	ImportedSkeleton.Release();
	ImportedModels.Clear(true);
	ImportedAnimations.Clear(true);
}


void RpgAssetTask_ImportModel::Execute() noexcept
{
	RPG_Check(SourceFilePath.IsFilePath());

	Assimp::Importer assimpImporter;
	assimpImporter.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, Scale);
	assimpImporter.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
	assimpImporter.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	assimpImporter.SetPropertyBool(AI_CONFIG_PP_FD_CHECKAREA, false);

	uint32_t flags =
		aiProcess_ConvertToLeftHanded |
		aiProcess_GlobalScale |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_FindDegenerates | aiProcess_SortByPType |
		//aiProcess_FindInvalidData |
		aiProcess_ValidateDataStructure;

	if (bImportSkeleton || bImportAnimation)
	{
		assimpImporter.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 8);
		flags |= aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData;
	}

	const aiScene* assimpScene = assimpImporter.ReadFile(*SourceFilePath, flags);

	// extract materials and textures
	ExtractMaterialTextures(assimpScene);

	// extract meshes
	ExtractMeshesFromNode(assimpScene, assimpScene->mRootNode);

	// extract animations
	ExtractAnimations(assimpScene);

	// wait all import texture tasks
	RPG_THREAD_TASK_WaitAll(ImportTextureTasks.GetData(), ImportTextureTasks.GetCount());

	RpgArray<RpgSharedTexture2D> ImportedTextures(ImportTextureTasks.GetCount());

	for (int t = 0; t < ImportedTextures.GetCount(); ++t)
	{
		ImportedTextures[t] = ImportTextureTasks[t]->GetResult();
	}


	// process intermediate models
	const RpgSharedMaterial& defaultMaterialMeshPhong = RpgMaterial::s_GetDefault(RpgMaterialDefault::MESH_PHONG);

	ImportedModels.Resize(IntermediateModels.GetCount());

	for (int i = 0; i < IntermediateModels.GetCount(); ++i)
	{
		const RpgAssimp::FModel& intModel = IntermediateModels[i];
		RpgSharedModel model = RpgModel::s_CreateShared(intModel.Name);
		model->AddLod();

		for (int j = 0; j < intModel.Meshes.GetCount(); ++j)
		{
			const RpgSharedMesh& mesh = intModel.Meshes[j];
			model->AddMesh(mesh);

			// setup material
			if (IntermediateMaterialPhongs.IsEmpty())
			{
				// Set to default
				model->SetMaterial(j, defaultMaterialMeshPhong);
				continue;
			}

			const int materialIndex = intModel.Materials[j];
			const RpgAssimp::FMaterialPhong& intMat = IntermediateMaterialPhongs[materialIndex];

			RpgSharedMaterial material = RpgMaterial::s_CreateSharedInstance(intMat.Name, defaultMaterialMeshPhong);
			material->SetParameterVectorValue("base_color", intMat.ParamVectorBaseColor);
			material->SetParameterVectorValue("specular_color", intMat.ParamVectorSpecularColor);
			material->SetParameterScalarValue("shininess", intMat.ParamScalarShininess);
			material->SetParameterScalarValue("opacity", intMat.ParamScalarOpacity);

			if (intMat.TextureIndexBaseColor != RPG_INDEX_INVALID)
			{
				material->SetParameterTextureValue(RpgMaterialParameterTexture::BASE_COLOR, ImportedTextures[intMat.TextureIndexBaseColor]);
			}

			if (intMat.TextureIndexNormal != RPG_INDEX_INVALID)
			{
				material->SetParameterTextureValue(RpgMaterialParameterTexture::NORMAL, ImportedTextures[intMat.TextureIndexNormal]);
			}

			if (intMat.TextureIndexSpecular != RPG_INDEX_INVALID)
			{
				material->SetParameterTextureValue(RpgMaterialParameterTexture::SPECULAR, ImportedTextures[intMat.TextureIndexSpecular]);
			}

			model->SetMaterial(j, material);
		}

		ImportedModels[i] = model;
	}


	// Cleanup import texture tasks
	for (int i = 0; i < ImportTextureTasks.GetCount(); ++i)
	{
		delete ImportTextureTasks[i];
	}
}


void RpgAssetTask_ImportModel::ExtractMaterialTextures(const aiScene* assimpScene)
{
	if (!bImportMaterialTexture)
	{
		return;
	}

	const int assimpMaterialCount = static_cast<int>(assimpScene->mNumMaterials);
	RPG_Check(assimpMaterialCount > 0);

	constexpr aiTextureType PHONG_TEXTURE_TYPES[] =
	{
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS,
		aiTextureType_SPECULAR,
	};

	constexpr int PHONG_TEXTURE_TYPE_COUNT = sizeof(PHONG_TEXTURE_TYPES) / sizeof(aiTextureType);

	RpgArray<RpgFilePath, 8> importingExternalTextures;
	RpgArray<int, 8> importingEmbeddedTextures;

	for (int m = 0; m < assimpMaterialCount; ++m)
	{
		const aiMaterial* assimpMaterial = assimpScene->mMaterials[m];

		RpgAssimp::FMaterialPhong& mat = IntermediateMaterialPhongs.Add();
		mat.Name = assimpMaterial->GetName().C_Str();

		int shadingMode;
		aiReturn ret = assimpMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingMode);

		// TODO: Setup different material params based on shading mode

		aiVector3D colorDiffuse;
		ret = assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
		mat.ParamVectorBaseColor = RpgVector4(colorDiffuse.x, colorDiffuse.y, colorDiffuse.z, 0.0f);

		aiVector3D colorSpecular;
		ret = assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
		mat.ParamVectorSpecularColor = RpgVector4(colorSpecular.x, colorSpecular.y, colorSpecular.z, 0.0f);
		ret = assimpMaterial->Get(AI_MATKEY_SHININESS, mat.ParamScalarShininess);
		ret = assimpMaterial->Get(AI_MATKEY_OPACITY, mat.ParamScalarOpacity);

		mat.TextureIndexBaseColor = RPG_INDEX_INVALID;
		mat.TextureIndexNormal = RPG_INDEX_INVALID;
		mat.TextureIndexSpecular = RPG_INDEX_INVALID;

		// textures
		for (int t = 0; t < PHONG_TEXTURE_TYPE_COUNT; ++t)
		{
			const aiTextureType assimpTextureType = PHONG_TEXTURE_TYPES[t];

			if (bIgnoreTextureNormals && assimpTextureType == aiTextureType_NORMALS)
			{
				continue;
			}

			const int assimpMaterialTextureCount = assimpMaterial->GetTextureCount(assimpTextureType);
			RPG_Check(assimpMaterialTextureCount <= 1);

			if (assimpMaterialTextureCount == 0)
			{
				continue;
			}

			aiString filePath;
			if (assimpMaterial->Get(AI_MATKEY_TEXTURE(assimpTextureType, 0), filePath) != AI_SUCCESS)
			{
				continue;
			}

			int textureIndex = ImportTextureTasks.GetCount();
			RpgAssetTask_ImportTexture* task = nullptr;
			RpgFilePath absoluteFilePath;

			// embedded texture
			if (filePath.data[0] == '*')
			{
				const int assimpTextureIndex = RpgPlatformString::CStringToInt(filePath.data + 1);
				RPG_Check(assimpTextureIndex >= 0 && assimpTextureIndex < 32);

				const aiTexture* assimpTexture = assimpScene->mTextures[assimpTextureIndex];

				RpgName sourceEmbeddedName = assimpTexture->mFilename.C_Str();
				if (sourceEmbeddedName.IsEmpty())
				{
					sourceEmbeddedName = RpgName::Format("TEX2D_%s_%i", *SourceFilePath.GetFileName(), assimpTextureIndex);
				}

				// check if already importing embedded texture
				const int importingIndex = importingEmbeddedTextures.FindIndexByValue(assimpTextureIndex);
				if (importingIndex == RPG_INDEX_INVALID)
				{
					importingEmbeddedTextures.AddValue(assimpTextureIndex);

					task = new RpgAssetTask_ImportTexture();
					task->Reset();
					task->SourceEmbedded = RpgAssimp::FTextureEmbedded(sourceEmbeddedName, assimpTexture->pcData, assimpTexture->mWidth, assimpTexture->mHeight, assimpTexture->achFormatHint);
				}
				else
				{
					RPG_Log(RpgLogAssetImporter, "Ignore import embedded texture (%s). Has been added to import list!", *sourceEmbeddedName);
					textureIndex = importingIndex;
				}
			}
			// external texture
			else
			{
				const RpgString directoryPath = SourceFilePath.GetDirectoryPath();
				const RpgFilePath relativeFilePath(RpgString(filePath.C_Str()));
				const RpgFilePath absoluteFilePath = directoryPath + relativeFilePath.ToString();

				// check if already importing external texture
				const int importingIndex = importingExternalTextures.FindIndexByValue(absoluteFilePath);
				if (importingIndex == RPG_INDEX_INVALID)
				{
					importingExternalTextures.AddValue(absoluteFilePath);

					task = new RpgAssetTask_ImportTexture();
					task->Reset();
					task->SourceFilePath = absoluteFilePath;
				}
				else
				{
					RPG_Log(RpgLogAssetImporter, "Ignore import external texture (%s). Has been added to import list!", *absoluteFilePath);
					textureIndex = importingIndex;
				}
			}

			if (assimpTextureType == aiTextureType_DIFFUSE)
			{
				mat.TextureIndexBaseColor = textureIndex;
			}
			else if (assimpTextureType == aiTextureType_NORMALS)
			{
				mat.TextureIndexNormal = textureIndex;
			}
			else if (assimpTextureType == aiTextureType_SPECULAR)
			{
				mat.TextureIndexSpecular = textureIndex;
			}
			else
			{
				RPG_NotImplementedYet();
			}

			// new import texture task
			if (task)
			{
				task->Format = RpgAssimp::ToTextureFormat(assimpTextureType);
				task->bGenerateMipMaps = bGenerateTextureMipMaps;
				ImportTextureTasks.AddValue(task);
			}
		}
	}

	if (!ImportTextureTasks.IsEmpty())
	{
		RpgThreadPool::SubmitTasks(reinterpret_cast<RpgThreadTask**>(ImportTextureTasks.GetData()), ImportTextureTasks.GetCount());
	}
}


void RpgAssetTask_ImportModel::ExtractSkeleton(const aiMesh* assimpMesh) noexcept
{
	const int assimpBoneCount = static_cast<int>(assimpMesh->mNumBones);
	if (assimpBoneCount == 0)
	{
		return;
	}

	RPG_Check(assimpBoneCount <= RPG_SKELETON_MAX_BONE);

	RpgAssimp::FSkeleton tempSkel;
	tempSkel.BoneNames.Reserve(assimpBoneCount);
	tempSkel.BoneParentIndices.Reserve(assimpBoneCount);
	tempSkel.BoneLocalTransforms.Reserve(assimpBoneCount);
	tempSkel.BoneInverseBindPoseTransforms.Reserve(assimpBoneCount);

	const aiNode* assimpArmatureNode = assimpMesh->mBones[0]->mArmature;
	RPG_Check(assimpArmatureNode);

	tempSkel.BoneNames.AddValue(assimpArmatureNode->mName.C_Str());
	tempSkel.BoneParentIndices.AddValue(RPG_SKELETON_BONE_INDEX_INVALID);
	tempSkel.BoneLocalTransforms.AddValue(RpgMatrixTransform());
	tempSkel.BoneInverseBindPoseTransforms.AddValue(RpgMatrixTransform());

	for (int b = 0; b < assimpBoneCount; ++b)
	{
		const aiBone* assimpBone = assimpMesh->mBones[b];

		tempSkel.BoneNames.AddValue(assimpBone->mName.C_Str());
		tempSkel.BoneParentIndices.AddValue(RPG_SKELETON_BONE_INDEX_INVALID);
		tempSkel.BoneLocalTransforms.AddValue(RpgAssimp::ToMatrixTransform(assimpBone->mNode->mTransformation));
		tempSkel.BoneInverseBindPoseTransforms.AddValue(RpgAssimp::ToMatrixTransform(assimpBone->mOffsetMatrix));
	}

	auto LocalFunc_UpdateSkeletonBoneHierarchy = [](RpgAssimp::FSkeleton& out_Skeleton, int boneIndex, const aiNode* assimpBoneNode)
	{
		const aiNode* assimpParentBoneNode = assimpBoneNode->mParent;

		if (assimpParentBoneNode == nullptr)
		{
			out_Skeleton.BoneParentIndices[boneIndex] = RPG_SKELETON_BONE_INDEX_INVALID;
			return;
		}

		const int boneParentIndex = out_Skeleton.GetBoneIndex(assimpParentBoneNode->mName.C_Str());
		RPG_Check(boneParentIndex != RPG_SKELETON_BONE_INDEX_INVALID && boneParentIndex < boneIndex);
		out_Skeleton.BoneParentIndices[boneIndex] = boneParentIndex;
	};

	for (int b = 0; b < assimpBoneCount; ++b)
	{
		LocalFunc_UpdateSkeletonBoneHierarchy(tempSkel, b + 1, assimpMesh->mBones[b]->mNode);
	}

	ImportedSkeleton = RpgAnimationSkeleton::s_CreateShared("SKEL_test");

	for (int b = 0; b < tempSkel.BoneNames.GetCount(); ++b)
	{
		ImportedSkeleton->AddBone(tempSkel.BoneNames[b], tempSkel.BoneParentIndices[b], tempSkel.BoneLocalTransforms[b], tempSkel.BoneInverseBindPoseTransforms[b]);
	}

	ImportedSkeleton->UpdateBindPoseTransforms();
}


void RpgAssetTask_ImportModel::ExtractMeshesFromNode(const aiScene* assimpScene, const aiNode* assimpNode) noexcept
{
	if (!ImportedSkeleton)
	{
		for (uint32_t m = 0; m < assimpNode->mNumMeshes; ++m)
		{
			const uint32_t meshIndex = assimpNode->mMeshes[m];
			const aiMesh* assimpMesh = assimpScene->mMeshes[meshIndex];
			const int assimpBoneCount = static_cast<int>(assimpMesh->mNumBones);
			RPG_Check(assimpBoneCount <= RPG_SKELETON_MAX_BONE);

			if (assimpBoneCount > 0)
			{
				ExtractSkeleton(assimpMesh);
				break;
			}
		}
	}

	if (assimpNode->mNumMeshes > 0)
	{
		RpgAssimp::FModel& model = IntermediateModels.Add();
		model.Name = assimpNode->mName.C_Str();

		RpgVertexMeshPositionArray tempVertexPositions;
		RpgVertexMeshNormalTangentArray tempVertexNormalTangents;
		RpgVertexMeshTexCoordArray tempVertexTexCoords;
		RpgVertexMeshSkinArray tempVertexSkins;
		RpgVertexIndexArray tempIndices;

		for (uint32_t m = 0; m < assimpNode->mNumMeshes; ++m)
		{
			RpgSharedMesh meshModel = RpgMesh::s_CreateShared(RpgName::Format("%s_mesh%i_lod0", *model.Name, m));

			const uint32_t meshIndex = assimpNode->mMeshes[m];
			const aiMesh* assimpMesh = assimpScene->mMeshes[meshIndex];

			const bool bHasVertexPosition = assimpMesh->HasPositions();
			const bool bHasNormal = assimpMesh->HasNormals();
			const bool bHasTangent = assimpMesh->HasTangentsAndBitangents();
			const bool bHasTexCoord = assimpMesh->HasTextureCoords(0);
			const bool bHasVertexIndex = assimpMesh->HasFaces();

			const int assimpVertexCount = static_cast<int>(assimpMesh->mNumVertices);
			RPG_Check(assimpVertexCount > 0);

			const int assimpIndexCount = static_cast<int>(assimpMesh->mNumFaces * 3);
			RPG_Check(assimpIndexCount > 0 && assimpIndexCount % 3 == 0);

			// Vertex position
			tempVertexPositions.Clear();
			tempVertexPositions.Resize(assimpVertexCount);

			for (int v = 0; v < assimpVertexCount; ++v)
			{
				const aiVector3D position = assimpMesh->mVertices[v];
				tempVertexPositions[v] = RpgVector4(position.x, position.y, position.z, 1.0f);
			}

			// Vertex normal, tangent
			tempVertexNormalTangents.Clear();
			tempVertexNormalTangents.Resize(assimpVertexCount);

			for (int v = 0; v < assimpVertexCount; ++v)
			{
				if (bHasNormal)
				{
					const aiVector3D normal = assimpMesh->mNormals[v];
					tempVertexNormalTangents[v].Normal = RpgVector4(normal.x, normal.y, normal.z, 0.0f);
				}

				if (bHasTangent)
				{
					const aiVector3D tangent = assimpMesh->mTangents[v];
					tempVertexNormalTangents[v].Tangent = RpgVector4(tangent.x, tangent.y, tangent.z, 0.0f);
				}
			}

			// Vertex texcoord
			tempVertexTexCoords.Clear();
			tempVertexTexCoords.Resize(assimpVertexCount);

			if (bHasTexCoord)
			{
				for (int v = 0; v < assimpVertexCount; ++v)
				{
					const aiVector3D texCoord = assimpMesh->mTextureCoords[0][v];
					tempVertexTexCoords[v] = DirectX::XMFLOAT2(texCoord.x, texCoord.y);
				}
			}
			

			// Vertex skin
			const int assimpBoneCount = static_cast<int>(assimpMesh->mNumBones);
			RPG_Check(assimpBoneCount <= RPG_SKELETON_MAX_BONE);

			if (assimpBoneCount > 0)
			{
				RPG_Check(ImportedSkeleton);

				tempVertexSkins.Clear();
				tempVertexSkins.Resize(assimpVertexCount);

				for (int b = 0; b < assimpBoneCount; ++b)
				{
					const aiBone* assimpBone = assimpMesh->mBones[b];

					const int boneIndex = ImportedSkeleton->GetBoneIndex(assimpBone->mName.C_Str());
					RPG_Check(boneIndex != RPG_SKELETON_BONE_INDEX_INVALID);

					const int assimpWeightCount = static_cast<int>(assimpBone->mNumWeights);
					for (int w = 0; w < assimpWeightCount; ++w)
					{
						const aiVertexWeight assimpVertexWeight = assimpBone->mWeights[w];
						const int vtxId = static_cast<int>(assimpVertexWeight.mVertexId);

						RpgVertex::FMeshSkin& vertexSkin = tempVertexSkins[vtxId];
						const uint8_t i = vertexSkin.BoneCount++;
						RPG_Check(i < 8);

						if (i < 4)
						{
							vertexSkin.BoneIndices0[i] = static_cast<uint8_t>(boneIndex);
							vertexSkin.BoneWeights0[i] = assimpVertexWeight.mWeight;
						}
						else
						{
							vertexSkin.BoneIndices1[i - 4] = static_cast<uint8_t>(boneIndex);
							vertexSkin.BoneWeights1[i - 4] = assimpVertexWeight.mWeight;
						}
					}
				}

				// Validate vertex skins, must have no influence from bone index 0 (Armature)
				for (int v = 0; v < assimpVertexCount; ++v)
				{
					const RpgVertex::FMeshSkin& skin = tempVertexSkins[v];
					for (int b = 0; b < skin.BoneCount; ++b)
					{
						if (b < 4)
						{
							RPG_Check(skin.BoneIndices0[b] != 0);
						}
						else
						{
							RPG_Check(skin.BoneIndices1[b - 4] != 0);
						}
					}
				}
			}

			// Vertex index
			tempIndices.Clear();
			tempIndices.Resize(assimpIndexCount);

			const int assimpFaceCount = static_cast<int>(assimpMesh->mNumFaces);
			int idx = 0;

			for (int f = 0; f < assimpFaceCount; ++f)
			{
				const aiFace& assimpFace = assimpMesh->mFaces[f];
				RPG_Check(assimpFace.mNumIndices == 3);
				RpgPlatformMemory::MemCopy(tempIndices.GetData() + idx, assimpFace.mIndices, sizeof(uint32_t) * assimpFace.mNumIndices);
				idx += assimpFace.mNumIndices;
			}

			meshModel->UpdateVertexData(assimpVertexCount, tempVertexPositions.GetData(), tempVertexNormalTangents.GetData(), tempVertexTexCoords.GetData(), tempVertexSkins.GetData(), assimpIndexCount, tempIndices.GetData());

			model.Meshes.AddValue(meshModel);
			model.Materials.AddValue(static_cast<int>(assimpMesh->mMaterialIndex));
		}
	}
	
	for (uint32_t c = 0; c < assimpNode->mNumChildren; ++c)
	{
		ExtractMeshesFromNode(assimpScene, assimpNode->mChildren[c]);
	}
}


void RpgAssetTask_ImportModel::ExtractAnimations(const aiScene* assimpScene) noexcept
{
	if (!bImportAnimation)
	{
		return;
	}

	const int assimpAnimCount = static_cast<int>(assimpScene->mNumAnimations);

	for (int i = 0; i < assimpAnimCount; ++i)
	{
		const aiAnimation* assimpAnimation = assimpScene->mAnimations[i];

		RpgName clipName = assimpAnimation->mName.C_Str();
		if (clipName.IsEmpty())
		{
			clipName = RpgName::Format("ANIM_import_%i", i);
		}

		RPG_Check(assimpAnimation->mTicksPerSecond > 0.0);
		const float durationInSeconds = static_cast<float>(assimpAnimation->mDuration / assimpAnimation->mTicksPerSecond);
		RpgSharedAnimationClip animClip = RpgAnimationClip::s_CreateShared(clipName, durationInSeconds);

		const int assimpAnimChannelCount = static_cast<int>(assimpAnimation->mNumChannels);
		for (int c = 0; c < assimpAnimChannelCount; ++c)
		{
			const aiNodeAnim* assimpNodeAnim = assimpAnimation->mChannels[c];

			RpgAnimationTrack track;
			track.BoneName = assimpNodeAnim->mNodeName.C_Str();

			// position
			const int assimpKeyPositionCount = static_cast<int>(assimpNodeAnim->mNumPositionKeys);
			for (int p = 0; p < assimpKeyPositionCount; ++p)
			{
				const aiVectorKey assimpKeyPosition = assimpNodeAnim->mPositionKeys[p];

				RpgAnimationTrack::FKeyPosition& keyPosition = track.KeyPositions.Add();
				keyPosition.Timestamp = static_cast<float>(assimpKeyPosition.mTime / assimpAnimation->mTicksPerSecond);
				keyPosition.Value = RpgAssimp::ToVector3(assimpKeyPosition.mValue);
			}

			// rotation
			const int assimpKeyRotationCount = static_cast<int>(assimpNodeAnim->mNumRotationKeys);
			for (int r = 0; r < assimpKeyRotationCount; ++r)
			{
				const aiQuatKey assimpKeyRotation = assimpNodeAnim->mRotationKeys[r];

				RpgAnimationTrack::FKeyRotation& keyRotation = track.KeyRotations.Add();
				keyRotation.Timestamp = static_cast<float>(assimpKeyRotation.mTime / assimpAnimation->mTicksPerSecond);
				keyRotation.Value = RpgAssimp::ToQuaternion(assimpKeyRotation.mValue);
			}

			animClip->AddTrack(track);
		}

		ImportedAnimations.AddValue(animClip);
	}
}
