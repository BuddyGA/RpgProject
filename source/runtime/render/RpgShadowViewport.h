#pragma once

#include "RpgRenderResource.h"
#include "task/RpgRenderTask_RenderPass.h"



class RpgShadowViewport
{
	RPG_NOCOPY(RpgShadowViewport)

public:
	// Game object id of this point light
	RpgGameObjectID GameObject;

	// Attenuation radius
	float AttenuationRadius;

	float SpotInnerConeDegree;
	float SpotOuterConeDegree;


public:
	RpgShadowViewport() noexcept
	{
		AttenuationRadius = 0.0f;
		SpotInnerConeDegree = 0.0f;
		SpotOuterConeDegree = 0.0f;
	}


	virtual ~RpgShadowViewport() noexcept = default;
	virtual void PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world, RpgWorldResource::FLightID lightId) noexcept = 0;
	virtual void SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses) noexcept = 0;
	virtual RpgSharedTexture2D GetTextureDepth(int frameIndex) noexcept = 0;

};



class RpgShadowViewport_PointLight : public RpgShadowViewport
{
public:
	RpgShadowViewport_PointLight() noexcept;

	virtual void PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world, RpgWorldResource::FLightID lightId) noexcept override;
	virtual void SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses) noexcept override;
	
	
	virtual RpgSharedTexture2D GetTextureDepth(int frameIndex) noexcept override
	{
		return FrameDatas[frameIndex].TextureDepthCube.Cast<RpgTexture2D>();
	}


private:
	struct FViewInfo
	{
		RpgMatrixTransform ViewMatrix;
		RpgWorldResource::FViewID ViewId{ RPG_INDEX_INVALID };
	};

	// [0: +X] [1: -X] [2: +Y] [3: -Y] [4: +Z] [5: -Z]
	FViewInfo FaceViews[6];


	struct FFrameData
	{
		RpgSharedTextureDepthCube TextureDepthCube;
		RpgArray<RpgDrawIndexedDepth> DrawMeshes;
		RpgArray<RpgDrawIndexedDepth> DrawSkinnedMeshes;
		RpgRenderTask_RenderPassShadow TaskRenderPassShadow;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];

};



class RpgShadowViewport_SpotLight : public RpgShadowViewport
{
public:
	RpgShadowViewport_SpotLight() noexcept;

	virtual void PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world, RpgWorldResource::FLightID lightId) noexcept override;
	virtual void SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses) noexcept override;


	virtual RpgSharedTexture2D GetTextureDepth(int frameIndex) noexcept override
	{
		return FrameDatas[frameIndex].TextureDepth;
	}


private:
	RpgWorldResource::FViewID ViewId;


	struct FFrameData
	{
		RpgSharedTexture2D TextureDepth;
		RpgArray<RpgDrawIndexedDepth> DrawMeshes;
		RpgArray<RpgDrawIndexedDepth> DrawSkinnedMeshes;
		RpgRenderTask_RenderPassShadow TaskRenderPassShadow;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];

};
