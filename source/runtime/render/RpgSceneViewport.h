#pragma once

#include "core/RpgMath.h"
#include "asset/RpgTexture.h"
#include "RpgRenderResource.h"
#include "task/RpgRenderTask_RenderPass.h"


class RpgWorld;
class RpgTerrain;



class RpgSceneViewport
{
	RPG_NOCOPY(RpgSceneViewport)

public:
	RpgWorld* World;
	bool bFrustumCulling;
	bool bWireframeMode;


public:
	RpgSceneViewport() noexcept;
	void UpdateFrame(RpgRenderFrameContext& context) noexcept;
	void SetupRenderPasses(const RpgRenderFrameContext& context, RpgRenderTask_RenderPass_Forward_Array& out_ForwardPasses) noexcept;


	inline void SetFrameRenderTargetDimension(int frameIndex, RpgPointInt in_Dimension) noexcept
	{
		FrameDatas[frameIndex].RenderTargetDimension = in_Dimension;
	}

	inline void SetFrameViewRotationAndPosition(int frameIndex, const RpgQuaternion& in_Rotation, const RpgVector3& in_Position) noexcept
	{
		FFrameData& frame = FrameDatas[frameIndex];
		frame.ViewRotation = in_Rotation;
		frame.ViewPosition = in_Position;
	}

	inline void GetFrameViewRotationAndPosition(int frameIndex, RpgQuaternion& out_Rotation, RpgVector3& out_Position) const noexcept
	{
		const FFrameData& frame = FrameDatas[frameIndex];
		out_Rotation = frame.ViewRotation;
		out_Position = frame.ViewPosition;
	}


	inline void SetFrameProjectionPerspective(int frameIndex, float in_FovDegree, float in_NearClipZ, float in_FarClipZ) noexcept
	{
		FFrameData& frame = FrameDatas[frameIndex];
		frame.FovDegree = in_FovDegree;
		frame.NearClipZ = in_NearClipZ;
		frame.FarClipZ = in_FarClipZ;
		frame.bOrthographicProjection = false;
	}


	inline void SetFrameProjectionOrthographic(int frameIndex, float in_NearClipZ, float in_FarClipZ) noexcept
	{
		FFrameData& frame = FrameDatas[frameIndex];
		frame.NearClipZ = in_NearClipZ;
		frame.FarClipZ = in_FarClipZ;
		frame.bOrthographicProjection = true;
	}


	inline const RpgBoundingFrustum& GetFrameViewFrustum(int frameIndex) const noexcept
	{
		return FrameDatas[frameIndex].ViewFrustum;
	}


	inline const RpgSharedTextureRenderTarget& GetFrameTextureRenderTarget(int frameIndex) const noexcept
	{
		return FrameDatas[frameIndex].TextureRenderTarget;
	}


	inline const RpgSharedTextureDepthStencil& GetFrameTextureDepthStencil(int frameIndex) const noexcept
	{
		return FrameDatas[frameIndex].TextureDepthStencil;
	}


	inline void AddFrameCapturedMesh(int frameIndex, const RpgSceneMesh& mesh) noexcept
	{
		FrameDatas[frameIndex].CapturedMeshes.AddValue(mesh);
	}


private:
	struct FFrameData
	{
		RpgMatrixTransform ViewMatrix;
		RpgMatrixProjection ProjectionMatrix;
		RpgQuaternion ViewRotation;
		RpgVector3 ViewPosition;
		RpgBoundingFrustum ViewFrustum;
		float FovDegree;
		float NearClipZ;
		float FarClipZ;
		bool bOrthographicProjection;
		RpgPointInt RenderTargetDimension;
		RpgSharedTextureRenderTarget TextureRenderTarget;
		RpgSharedTextureDepthStencil TextureDepthStencil;

		RpgArray<RpgSceneMesh> CapturedMeshes;

		RpgRenderTask_RenderPass_Forward TaskRenderPassForward;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];

};
