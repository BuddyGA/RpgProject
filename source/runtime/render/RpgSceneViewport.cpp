#include "RpgSceneViewport.h"
#include "core/world/RpgWorld.h"
#include "animation/world/RpgAnimationComponent.h"
#include "RpgShadowViewport.h"



RpgSceneViewport::RpgSceneViewport() noexcept
	: FrameDatas()
{
	RenderTargetDimension = RpgPointInt(1600, 900);
	FovDegree = 90.0f;
	NearClipZ = 10.0f;
	FarClipZ = 10000.0f;
	bOrthographicProjection = false;
	bDirtyProjection = true;


#ifndef RPG_BUILD_SHIPPING
	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FFrameDebug& debug = FrameDebugs[f];
		debug.LineMaterialId = RPG_INDEX_INVALID;
		debug.CameraId = RPG_INDEX_INVALID;
	}
#endif // !RPG_BUILD_SHIPPING

}


void RpgSceneViewport::PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];
	frame.DrawOpaqueMeshes.Clear();
	frame.DrawOpaqueSkinnedMeshes.Clear();
	frame.DrawTransparencies.Clear();


	// Resize render target
	{
		if (!frame.TextureRenderTarget)
		{
			frame.TextureRenderTarget = RpgTexture2D::s_CreateSharedRenderTarget("TEXRT_ScnVprt", RpgTextureFormat::TEX_RT_RGBA, RenderTargetDimension.X, RenderTargetDimension.Y);
		}

		frame.TextureRenderTarget->Resize(RenderTargetDimension.X, RenderTargetDimension.Y);
		frame.TextureRenderTarget->GPU_UpdateResource();


		// Resize depth-stencil
		if (!frame.TextureDepthStencil)
		{
			frame.TextureDepthStencil = RpgTexture2D::s_CreateSharedDepthStencil("TEXDS_ScnVprt", RpgTextureFormat::TEX_DS_32, RenderTargetDimension.X, RenderTargetDimension.Y);
		}

		frame.TextureDepthStencil->Resize(RenderTargetDimension.X, RenderTargetDimension.Y);
		frame.TextureDepthStencil->GPU_UpdateResource();
	}


	// Update view projection
	UpdateViewProjection();


	// Build draw calls
	RpgMaterialResource* materialResource = frameContext.MaterialResource;
	RpgMeshResource* meshResource = frameContext.MeshResource;
	RpgMeshSkinnedResource* meshSkinnedResource = frameContext.MeshSkinnedResource;

	const RpgWorldResource::FViewID cameraId = worldResource->AddView(ViewMatrix, ProjectionMatrix, ViewPosition, NearClipZ, FarClipZ);

	RpgArray<RpgMatrixTransform> tempBoneSkinningTransforms;

	for (int m = 0; m < Meshes.GetCount(); ++m)
	{
		const RpgSceneMesh& data = Meshes[m];

		const bool bHasSkin = data.Mesh->HasSkin();
		bool bIsStaticMesh = true;

		RpgDrawIndexed draw;
		draw.Material = data.Material ? materialResource->AddMaterial(data.Material) : materialResource->AddMaterial(RpgMaterial::s_GetDefault(RpgMaterialDefault::MESH_PHONG));
		draw.ObjectParam.ViewIndex = cameraId;
		draw.ObjectParam.TransformIndex = worldResource->AddTransform(data.GameObject.GetIndex(), data.WorldTransformMatrix);

		if (bHasSkin)
		{
			const RpgAnimationComponent_AnimSkeletonPose* animComp = world->GameObject_GetComponent<RpgAnimationComponent_AnimSkeletonPose>(data.GameObject);

			// draw as static mesh if no animation component
			bIsStaticMesh = (animComp == nullptr);

			if (animComp)
			{
				const RpgAnimationSkeleton* skeleton = animComp->GetSkeleton().Get();
				RPG_Check(skeleton);

				const int boneCount = skeleton->GetBoneCount();
				tempBoneSkinningTransforms.Resize(boneCount);

				for (int b = 0; b < boneCount; ++b)
				{
					tempBoneSkinningTransforms[b] = skeleton->GetBoneInverseBindPoseTransform(b) * animComp->GetFinalPose().GetBonePoseTransform(b);
				}

				const RpgMeshSkinnedResource::FMeshID meshId = meshSkinnedResource->AddMesh(data.Mesh, draw.IndexCount, draw.IndexStart, draw.IndexVertexOffset);
				meshSkinnedResource->AddObjectBoneSkinningTransforms(meshId, tempBoneSkinningTransforms);
			}
		}

		if (bIsStaticMesh)
		{
			meshResource->AddMesh(data.Mesh, draw.IndexCount, draw.IndexStart, draw.IndexVertexOffset);
		}

		if (data.Material->IsTransparency())
		{
			frame.DrawTransparencies.AddValue(draw);
		}
		else
		{
			if (bIsStaticMesh)
			{
				frame.DrawOpaqueMeshes.AddValue(draw);
			}
			else
			{
				frame.DrawOpaqueSkinnedMeshes.AddValue(draw);
			}
		}
	}


	for (int l = 0; l < Lights.GetCount(); ++l)
	{
		const RpgSceneLight& data = Lights[l];
		RpgWorldResource::FLightID lightId = RPG_INDEX_INVALID;

		if (data.Type == RpgRenderLight::TYPE_POINT_LIGHT)
		{
			lightId = worldResource->AddLight_Point(data.GameObject.GetIndex(), data.WorldTransform.Position, data.ColorIntensity, data.AttenuationRadius, data.AttenuationFallOffExp);
		}
		else if (data.Type == RpgRenderLight::TYPE_SPOT_LIGHT)
		{
			lightId = worldResource->AddLight_Spot(data.GameObject.GetIndex(), data.WorldTransform.Position, data.WorldTransform.GetAxisForward(), 
				data.ColorIntensity, data.AttenuationRadius, 16, data.SpotInnerConeDegree, data.SpotOuterConeDegree);
		}
		else
		{
			RPG_NotImplementedYet();
		}

		if (data.ShadowViewport)
		{
			data.ShadowViewport->PreRender(frameContext, worldResource, world, lightId);
		}
	}

#ifndef RPG_BUILD_SHIPPING
	FFrameDebug& debug = FrameDebugs[frameContext.Index];
	debug.LineMaterialId = materialResource->AddMaterial(RpgMaterial::s_GetDefault(RpgMaterialDefault::DEBUG_PRIMITIVE_LINE));
	debug.LineNoDepthMaterialId = materialResource->AddMaterial(RpgMaterial::s_GetDefault(RpgMaterialDefault::DEBUG_PRIMITIVE_LINE_NO_DEPTH));
	debug.CameraId = cameraId;
#endif // !RPG_BUILD_SHIPPING

}


void RpgSceneViewport::SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses, RpgRenderTask_RenderPassForwardArray& out_ForwardPasses) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];

	// shadow pass
	for (int i = 0; i < Lights.GetCount(); ++i)
	{
		if (Lights[i].ShadowViewport)
		{
			Lights[i].ShadowViewport->SetupRenderPasses(frameContext, worldResource, world, out_ShadowPasses);
		}
	}

	// forward pass
	RpgRenderTask_RenderPassForward* forwardPass = &frame.TaskRenderPassForward;
	forwardPass->Reset();
	forwardPass->FrameContext = frameContext;
	forwardPass->WorldResource = worldResource;
	forwardPass->TextureRenderTarget = frame.TextureRenderTarget.Get();
	forwardPass->TextureDepthStencil = frame.TextureDepthStencil.Get();
	forwardPass->DrawMeshData = frame.DrawOpaqueMeshes.GetData();
	forwardPass->DrawMeshCount = frame.DrawOpaqueMeshes.GetCount();
	forwardPass->DrawSkinnedMeshData = frame.DrawOpaqueSkinnedMeshes.GetData();
	forwardPass->DrawSkinnedMeshCount = frame.DrawOpaqueSkinnedMeshes.GetCount();

#ifndef RPG_BUILD_SHIPPING
	FFrameDebug& debug = FrameDebugs[frameContext.Index];
	forwardPass->DebugDrawLineMaterialId = debug.LineMaterialId;
	forwardPass->DebugDrawLineNoDepthMaterialId = debug.LineNoDepthMaterialId;
	forwardPass->DebugDrawCameraId = debug.CameraId;
#endif // !RPG_BUILD_SHIPPING

	out_ForwardPasses.AddValue(forwardPass);
}
