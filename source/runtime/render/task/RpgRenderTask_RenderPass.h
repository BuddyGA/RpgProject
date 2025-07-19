#pragma once

#include "core/RpgThreadPool.h"
#include "../RpgRenderResource.h"


class RpgTexture2D;



class RpgRenderTask_RenderPass : public RpgThreadTask
{

public:
	RpgRenderFrameContext FrameContext;
	const RpgWorldResource* WorldResource;
	

public:
	RpgRenderTask_RenderPass() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	inline ID3D12CommandList* GetCommandList() const noexcept
	{
		return CmdListDirect.Get();
	}


protected:
	virtual void CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept = 0;


private:
	ComPtr<ID3D12CommandAllocator> CmdAllocDirect;
	ComPtr<ID3D12GraphicsCommandList> CmdListDirect;

};



class RpgRenderTask_RenderPassShadow : public RpgRenderTask_RenderPass
{
public:
	RpgTexture2D* TextureDepth;
	RpgWorldResource::FViewID ViewId;

	const RpgDrawIndexedDepth* DrawMeshData;
	int DrawMeshCount;

	const RpgDrawIndexedDepth* DrawSkinnedMeshData;
	int DrawSkinnedMeshCount;

	bool bIsOmniDirectional;


public:
	RpgRenderTask_RenderPassShadow() noexcept;
	virtual void Reset() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_RenderPassShadow";
	}


protected:
	virtual void CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept override;

};



class RpgRenderTask_RenderPassForward : public RpgRenderTask_RenderPass
{
public:
	RpgTexture2D* TextureRenderTarget;
	RpgTexture2D* TextureDepthStencil;

	const RpgDrawIndexed* DrawMeshData;
	int DrawMeshCount;

	const RpgDrawIndexed* DrawSkinnedMeshData;
	int DrawSkinnedMeshCount;


public:
	RpgRenderTask_RenderPassForward() noexcept;
	virtual void Reset() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_RenderPassForward";
	}


protected:
	virtual void CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept override;


#ifndef RPG_BUILD_SHIPPING
public:
	RpgMaterialResource::FMaterialID DebugDrawLineMaterialId;
	RpgMaterialResource::FMaterialID DebugDrawLineNoDepthMaterialId;
	RpgWorldResource::FViewID DebugDrawCameraId;
#endif // !RPG_BUILD_SHIPPING

};
