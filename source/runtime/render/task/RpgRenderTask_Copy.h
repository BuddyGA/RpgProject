#pragma once

#include "core/RpgThreadPool.h"
#include "core/dsa/RpgArray.h"
#include "../RpgRenderTypes.h"



class RpgRenderTask_Copy : public RpgThreadTask
{
public:
	ID3D12Fence* FenceSignal;
	uint64_t FenceSignalValue;
	RpgRenderFrameContext FrameContext;
	RpgRenderer2D* Renderer2d;
	RpgArrayInline<RpgWorldResource*, 8> WorldResources;


public:
	RpgRenderTask_Copy() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_Copy";
	}


private:
	ComPtr<ID3D12CommandAllocator> CmdAllocCopy;
	ComPtr<ID3D12GraphicsCommandList> CmdListCopy;

};
