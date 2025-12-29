#pragma once

#include "core/RpgThreadPool.h"
#include "../RpgRenderResource.h"


class RpgTerrain;



class RpgRenderTask_RenderPass : public RpgThreadTask
{
public:
	const RpgRenderFrameContext* FrameContext;


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




typedef RpgArrayInline<class RpgRenderTask_RenderPass_Forward*, 8> RpgRenderTask_RenderPass_Forward_Array;

class RpgRenderTask_RenderPass_Forward : public RpgRenderTask_RenderPass
{
public:
	RpgTextureRenderTarget* TextureRenderTarget;
	RpgTextureDepthStencil* TextureDepthStencil;

	/*
	const RpgDrawIndexed* DrawMeshData;
	int DrawMeshCount;

	const RpgDrawIndexed* DrawSkinnedMeshData;
	int DrawSkinnedMeshCount;

	const RpgDrawIndexed* DrawTerrainData;
	int DrawTerrainCount;
	*/

	RpgArray<const RpgTerrain*> DrawTerrains;


public:
	RpgRenderTask_RenderPass_Forward() noexcept;
	virtual void Reset() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_RenderPass_Forward";
	}


protected:
	virtual void CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept override;

};
