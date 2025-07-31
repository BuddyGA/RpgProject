#include "RpgEngine.h"
#include "asset/RpgAssetImporter.h"
#include "core/world/RpgWorld.h"
#include "render/world/RpgRenderComponent.h"
#include "animation/world/RpgAnimationComponent.h"



static void TestLevel_AddBlocker(RpgWorld* world, RpgVector3 center, RpgVector3 halfExtents, float uvScale) noexcept
{
	RpgSharedModel model = RpgModel::s_CreateShared("MDL_DEF_floor");
	{
		model->AddLod();
		model->AddMeshEmpty();

		RpgVertexMeshPositionArray vertexPositions;
		RpgVertexMeshNormalTangentArray vertexNormalTangents;
		RpgVertexMeshTexCoordArray vertexTexCoords;
		RpgVertexIndexArray indices;
		RpgVertexGeometryFactory::CreateMeshBox(vertexPositions, vertexNormalTangents, vertexTexCoords, indices, -halfExtents, halfExtents, uvScale);

		RpgSharedMesh& mesh = model->GetMeshLod(0, 0);
		mesh->UpdateVertexData(vertexPositions.GetCount(), vertexPositions.GetData(), vertexNormalTangents.GetData(), vertexTexCoords.GetData(), nullptr, indices.GetCount(), indices.GetData());

		model->SetMaterial(0, RpgMaterial::s_GetDefault(RpgMaterialDefault::MESH_PHONG));
	}

	static int Counter = 0;

	const RpgGameObjectID blocker = world->GameObject_Create(RpgName::Format("test_blocker_%i", Counter++), RpgTransform(center));
	{
		// Add render component
		RpgRenderComponent_Mesh* meshComp = world->GameObject_AddComponent<RpgRenderComponent_Mesh>(blocker);
		meshComp->Model = model;
		meshComp->bIsVisible = true;
	}
}


static void TestLevel_AddBox(RpgWorld* world, const RpgTransform& transform) noexcept
{
	static RpgSharedModel BoxModel;

	if (!BoxModel)
	{
		BoxModel = RpgModel::s_CreateShared("MDL_DEF_box");
		BoxModel->AddLod();
		BoxModel->AddMeshEmpty();

		RpgVertexMeshPositionArray vertexPositions;
		RpgVertexMeshNormalTangentArray vertexNormalTangents;
		RpgVertexMeshTexCoordArray vertexTexCoords;
		RpgVertexIndexArray indices;
		RpgVertexGeometryFactory::CreateMeshBox(vertexPositions, vertexNormalTangents, vertexTexCoords, indices, RpgVector3(-64.0f), RpgVector3(64.0f), 2.0f);

		RpgSharedMesh& mesh = BoxModel->GetMeshLod(0, 0);
		mesh->UpdateVertexData(vertexPositions.GetCount(), vertexPositions.GetData(), vertexNormalTangents.GetData(), vertexTexCoords.GetData(), nullptr, indices.GetCount(), indices.GetData());

		BoxModel->SetMaterial(0, RpgMaterial::s_GetDefault(RpgMaterialDefault::MESH_PHONG));
	}

	static int Counter = 0;

	const RpgGameObjectID box = world->GameObject_Create(RpgName::Format("test_box_%i", Counter++), transform);
	{
		// Add render component
		RpgRenderComponent_Mesh* meshComp = world->GameObject_AddComponent<RpgRenderComponent_Mesh>(box);
		meshComp->Model = BoxModel;
		meshComp->bIsVisible = true;
	}
}


static void TestLevel_AddLight_Point(RpgWorld* world, const RpgTransform& transform, RpgColorLinear colorIntensity, float radius, bool bCastShadow) noexcept
{
	static int Counter = 0;

	RpgGameObjectID pointLight = world->GameObject_Create(RpgName::Format("test_pointlight_%i", Counter++), transform);
	{
		RpgRenderComponent_Light* lightComp = world->GameObject_AddComponent<RpgRenderComponent_Light>(pointLight);
		lightComp->Type = RpgRenderLight::TYPE_POINT_LIGHT;
		lightComp->ColorIntensity = colorIntensity;
		lightComp->AttenuationRadius = radius;
		lightComp->bIsVisible = true;
		lightComp->bCastShadow = bCastShadow;
	}
}


static void TestLevel_AddLight_Spot(RpgWorld* world, const RpgTransform& transform, RpgColorLinear colorIntensity, float radius, float innerConeDeg, float outerConeDeg, bool bCastShadow) noexcept
{
	static int Counter = 0;

	RpgGameObjectID pointLight = world->GameObject_Create(RpgName::Format("test_spotlight_%i", Counter++), transform);
	{
		RpgRenderComponent_Light* lightComp = world->GameObject_AddComponent<RpgRenderComponent_Light>(pointLight);
		lightComp->Type = RpgRenderLight::TYPE_SPOT_LIGHT;
		lightComp->ColorIntensity = colorIntensity;
		lightComp->AttenuationRadius = radius;
		lightComp->SpotInnerConeDegree = innerConeDeg;
		lightComp->SpotOuterConeDegree = outerConeDeg;
		lightComp->bIsVisible = true;
		lightComp->bCastShadow = bCastShadow;
	}
}


static void TestLevel_OBJ(RpgWorld* world, const RpgFilePath& sourceFilePath, float scale, bool bGenerateTextureMipMaps = false, bool bIgnoreTextureNormals = false) noexcept
{
	RpgAssetImportSetting_Model setting;
	setting.SourceFilePath = sourceFilePath;
	setting.Scale = scale;
	setting.bImportMaterialTexture = true;
	setting.bImportSkeleton = true;
	setting.bImportAnimation = true;
	setting.bGenerateTextureMipMaps = bGenerateTextureMipMaps;
	setting.bIgnoreTextureNormals = bIgnoreTextureNormals;

	RpgArray<RpgSharedModel> importedModels;
	RpgSharedAnimationSkeleton importedSkeleton;
	RpgArray<RpgSharedAnimationClip> importedAnimations;
	g_AssetImporter->ImportModel(importedModels, importedSkeleton, importedAnimations, setting);

	for (int i = 0; i < importedModels.GetCount(); ++i)
	{
		const RpgSharedModel& model = importedModels[i];

		RpgTransform transform;
		transform.Position = RpgVector3(0.0f, 0.0f, 0.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, 0.0f, 0.0f);

		RpgGameObjectID gameObject = world->GameObject_Create(model->GetName(), transform);
		RpgRenderComponent_Mesh* meshComp = world->GameObject_AddComponent<RpgRenderComponent_Mesh>(gameObject);
		meshComp->Model = model;
		meshComp->bIsVisible = true;

		/*
		if (model->HasSkin())
		{
			RPG_Check(importedSkeleton);
			RpgAnimationComponent_AnimSkeletonPose* animComp = world->GameObject_AddComponent<RpgAnimationComponent_AnimSkeletonPose>(gameObject);
			animComp->SetSkeleton(importedSkeleton);
			animComp->Clip = importedAnimations[0];
			animComp->PlayRate = 1.0f;
			animComp->bLoopAnim = true;
		}
		*/
	}

	g_AssetImporter->Reset();
}


static void TestLevel_Sponza(RpgWorld* world) noexcept
{
	TestLevel_OBJ(world, RpgFileSystem::GetAssetRawDirPath() + "model/sponza_phong/sponza.obj", 1.0f, false, true);

	RpgTransform transform;

	transform.Position = RpgVector3(650.0f, 200.0f, 0.0f);
	TestLevel_AddLight_Point(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 0.33f), 800.0f, true);

	transform.Position = RpgVector3(0.0f, 200.0f, 0.0f);
	TestLevel_AddLight_Point(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 0.33f), 800.0f, true);

	transform.Position = RpgVector3(-650.0f, 200.0f, 0.0f);
	TestLevel_AddLight_Point(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 0.33f), 800.0f, true);

	transform.Position = RpgVector3(645.0f, 750.0f, 60.0f);
	transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(20.0f, 90.0f, 0.0f);
	TestLevel_AddLight_Spot(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f), 1600.0f, 20.0f, 40.0f, true);
}


static void TestLevel_PrimitiveShapes(RpgWorld* world) noexcept
{
	TestLevel_AddBlocker(world, RpgVector3::ZERO, RpgVector3(2048.0f, 16.0f, 2048.0f), 16.0f);

	RpgTransform transform;

	// test point light
	{
		transform.Position = RpgVector3(0.0f, 500.0f, 0.0f);
		TestLevel_AddLight_Point(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f), 800.0f, true);

		// +X
		transform.Position = RpgVector3(300.0f, 500.0f, 0.0f);
		TestLevel_AddBox(world, transform);
		TestLevel_AddBlocker(world, RpgVector3(500.0f, 500.0f, 0.0f), RpgVector3(16, 200.0f, 200.0f), 4.0f);

		// -X
		transform.Position = RpgVector3(-300.0f, 500.0f, 0.0f);
		TestLevel_AddBox(world, transform);
		TestLevel_AddBlocker(world, RpgVector3(-500.0f, 500.0f, 0.0f), RpgVector3(16, 200.0f, 200.0f), 4.0f);
		
		// +Y
		transform.Position = RpgVector3(0.0f, 800.0f, 0.0f);
		TestLevel_AddBox(world, transform);
		TestLevel_AddBlocker(world, RpgVector3(0.0f, 1000.0f, 0.0f), RpgVector3(200.0f, 16.0f, 200.0f), 4.0f);
		
		// -Y
		transform.Position = RpgVector3(0.0f, 280.0f, 0.0f);
		TestLevel_AddBox(world, transform);

		// +Z
		transform.Position = RpgVector3(0.0f, 500.0f, 300.0f);
		TestLevel_AddBox(world, transform);
		TestLevel_AddBlocker(world, RpgVector3(0.0f, 500.0f, 500.0f), RpgVector3(200.0f, 200.0f, 16.0f), 4.0f);
		
		// -Z
		transform.Position = RpgVector3(0.0f, 500.0f, -300.0f);
		TestLevel_AddBox(world, transform);
		TestLevel_AddBlocker(world, RpgVector3(0.0f, 500.0f, -500.0f), RpgVector3(200.0f, 200.0f, 16.0f), 4.0f);
	}

	// test spot light
	{
		transform.Position = RpgVector3(-1000.0f, 555.0f, 300.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(60.0f, 0.0f, 0.0f);
		TestLevel_AddLight_Spot(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f), 1600.0f, 20.0f, 40.0f, true);

		transform.Position = RpgVector3(-900.0f, 400.0f, 500.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(40.0f, 0.0f, 0.0f);
		TestLevel_AddBox(world, transform);

		transform.Position = RpgVector3(-1100.0f, 80.0f, 500.0f);
		transform.Rotation = RpgQuaternion();
		TestLevel_AddBox(world, transform);

		transform.Position = RpgVector3(-1316.0f, 422.0f, 250.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(30.75f, 38.0f, 0.0f);
		TestLevel_AddLight_Spot(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f), 1600.0f, 20.0f, 40.0f, true);

		transform.Position = RpgVector3(-380.0f, 854.0f, -428.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(45.25f, 48.0f, 0.0f);
		TestLevel_AddLight_Spot(world, transform, RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f), 3200.0f, 20.0f, 40.0f, true);
	}
}



static void TestLevel_Animations(RpgWorld* world) noexcept
{
	RpgSharedModel models[2];
	RpgSharedAnimationSkeleton skeletons[2];
	RpgSharedAnimationClip animationClips[2];

	RpgAssetImportSetting_Model setting;
	setting.SourceFilePath = RpgFileSystem::GetAssetRawDirPath() + "model/CesiumMan.glb";
	setting.Scale = 100.0f;
	setting.bImportMaterialTexture = true;
	setting.bImportSkeleton = true;
	setting.bImportAnimation = true;
	setting.bGenerateTextureMipMaps = true;

	RpgArray<RpgSharedModel> importedModels;
	RpgSharedAnimationSkeleton importedSkeleton;
	RpgArray<RpgSharedAnimationClip> importedAnimations;
	g_AssetImporter->ImportModel(importedModels, importedSkeleton, importedAnimations, setting);
	models[0] = importedModels[0];
	skeletons[0] = importedSkeleton;
	animationClips[0] = importedAnimations[0];
	
	setting.SourceFilePath = RpgFileSystem::GetAssetRawDirPath() + "model/RiggedFigure.glb";
	g_AssetImporter->ImportModel(importedModels, importedSkeleton, importedAnimations, setting);
	models[1] = importedModels[0];
	skeletons[1] = importedSkeleton;
	animationClips[1] = importedAnimations[0];

	const int DIM_X = 16;
	const int DIM_Z = 16;
	const float OFFSET = 128.0f;
	const RpgVector3 startPos(-(DIM_X * OFFSET * 0.5f), 0.0f, -(DIM_Z * OFFSET * 0.5f));
	//const RpgVector3 startPos(500.0f, 20.0f, 0.0f);
	RpgVector3 spawnPos = startPos;
	int modelIndex = 0;

	for (int x = 0; x < DIM_X; ++x)
	{
		spawnPos.Z = startPos.Z;

		for (int z = 0; z < DIM_Z; ++z)
		{
			RpgTransform transform;
			transform.Position = spawnPos;
			transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(90.0f, 0.0f, 0.0f);

			RpgGameObjectID gameObject = world->GameObject_Create(RpgName::Format("test_%i_%i", x, z), transform);

			RpgRenderComponent_Mesh* meshComp = world->GameObject_AddComponent<RpgRenderComponent_Mesh>(gameObject);
			meshComp->Model = models[modelIndex];
			meshComp->bIsVisible = true;

			RpgAnimationComponent_AnimSkeletonPose* animComp = world->GameObject_AddComponent<RpgAnimationComponent_AnimSkeletonPose>(gameObject);
			animComp->SetSkeleton(skeletons[modelIndex]);
			animComp->Clip = animationClips[modelIndex];
			animComp->PlayRate = 1.5f;
			animComp->bLoopAnim = true;

			spawnPos.Z += OFFSET;
			modelIndex = (modelIndex + 1) % 2;
		}
		
		spawnPos.X += OFFSET;
	}
}


void RpgEngine::CreateTestLevel() noexcept
{
	//TestLevel_Sponza(MainWorld);

	//TestLevel_OBJ(MainWorld, RpgFileSystem::GetAssetRawDirPath() + "model/lost_empire/lost_empire.obj", 100.0f);
	//TestLevel_OBJ(MainWorld, RpgFileSystem::GetAssetRawDirPath() + "model/bunny/bunny.obj", 100.0f);
	//TestLevel_OBJ(MainWorld, RpgFileSystem::GetAssetRawDirPath() + "model/RiggedFigure.glb", 100.0f);
	//TestLevel_OBJ(MainWorld, RpgFileSystem::GetAssetRawDirPath() + "model/CesiumMilkTruck.glb", 100.0f);

	//TestLevel_Animations(MainWorld);

	TestLevel_PrimitiveShapes(MainWorld);
}

