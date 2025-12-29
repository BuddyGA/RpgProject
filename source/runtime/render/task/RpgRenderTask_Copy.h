#pragma once

#include "core/RpgThreadPool.h"
#include "../RpgRenderResource.h"



class RpgRenderTask_Copy : public RpgThreadTask
{
public:
	ID3D12Fence* FenceSignal;
	uint64_t FenceSignalValue;
	RpgRenderFrameContext* FrameContext;


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
