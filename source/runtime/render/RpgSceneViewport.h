#pragma once

#include "RpgRenderResource.h"
#include "task/RpgRenderTask_RenderPass.h"



class RpgSceneViewport
{
public:
	RpgPointInt RenderTargetDimension;
	


public:
	RpgSceneViewport() noexcept;

	void PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world) noexcept;
	void SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses, RpgRenderTask_RenderPassForwardArray& out_ForwardPasses) noexcept;


	inline void SetViewRotationAndPosition(const RpgQuaternion& in_Rotation, const RpgVector3& in_Position) noexcept
	{
		ViewRotation = in_Rotation;
		ViewPosition = in_Position;
	}

	inline void GetViewRotationAndPosition(RpgQuaternion& out_Rotation, RpgVector3& out_Position) const noexcept
	{
		out_Rotation = ViewRotation;
		out_Position = ViewPosition;
	}


	inline void SetProjectionPerspective(float in_FovDegree, float in_NearClipZ, float in_FarClipZ) noexcept
	{
		FovDegree = in_FovDegree;
		NearClipZ = in_NearClipZ;
		FarClipZ = in_FarClipZ;
		bOrthographicProjection = false;
		bDirtyProjection = true;
	}


	inline void SetProjectionOrthographic(float in_NearClipZ, float in_FarClipZ) noexcept
	{
		NearClipZ = in_NearClipZ;
		FarClipZ = in_FarClipZ;
		bOrthographicProjection = true;
		bDirtyProjection = true;
	}


	inline void UpdateViewProjection() noexcept
	{
		const RpgMatrixTransform worldMatrixTransform(ViewPosition, ViewRotation);

		ViewMatrix = worldMatrixTransform.GetInverse();

		ProjectionMatrix = bOrthographicProjection ?
			RpgMatrixProjection::CreateOrthographic(0.0f, static_cast<float>(RenderTargetDimension.X), 0.0f, static_cast<float>(RenderTargetDimension.Y), NearClipZ, FarClipZ) :
			RpgMatrixProjection::CreatePerspective(static_cast<float>(RenderTargetDimension.X) / static_cast<float>(RenderTargetDimension.Y), FovDegree, NearClipZ, FarClipZ);

		ViewFrustum.CreateFromMatrix(worldMatrixTransform, ProjectionMatrix);
	}


	inline const RpgBoundingFrustum& GetViewFrustum() const noexcept
	{
		return ViewFrustum;
	}

	inline const RpgSharedTexture2D& GetTextureRenderTarget(int frameIndex) const noexcept
	{
		return FrameDatas[frameIndex].TextureRenderTarget;
	}

	inline const RpgSharedTexture2D& GetTextureDepthStencil(int frameIndex) const noexcept
	{
		return FrameDatas[frameIndex].TextureDepthStencil;
	}

	inline RpgArray<RpgSceneMesh>& GetFrameMeshes(int frameIndex) noexcept
	{
		return FrameDatas[frameIndex].Meshes;
	}

	inline RpgArray<RpgSceneLight>& GetFrameLights(int frameIndex) noexcept
	{
		return FrameDatas[frameIndex].Lights;
	}


private:
	RpgMatrixTransform ViewMatrix;
	RpgMatrixProjection ProjectionMatrix;
	RpgQuaternion ViewRotation;
	RpgVector3 ViewPosition;
	RpgBoundingFrustum ViewFrustum;
	float FovDegree;
	float NearClipZ;
	float FarClipZ;
	bool bOrthographicProjection;
	bool bDirtyProjection;


	struct FFrameData
	{
		RpgSharedTexture2D TextureRenderTarget;
		RpgSharedTexture2D TextureDepthStencil;

		RpgArray<RpgSceneMesh> Meshes;
		RpgArray<RpgSceneLight> Lights;

		RpgArray<RpgDrawIndexed> DrawOpaqueMeshes;
		RpgArray<RpgDrawIndexed> DrawOpaqueSkinnedMeshes;

		RpgArray<RpgDrawIndexed> DrawTransparencies;

		RpgRenderTask_RenderPassForward TaskRenderPassForward;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];


#ifndef RPG_BUILD_SHIPPING
private:
	struct FFrameDebug
	{
		RpgMaterialResource::FMaterialID LineMaterialId;
		RpgMaterialResource::FMaterialID LineNoDepthMaterialId;
		RpgWorldResource::FViewID CameraId;
	};
	FFrameDebug FrameDebugs[RPG_FRAME_BUFFERING];
#endif // !RPG_BUILD_SHIPPING

};
