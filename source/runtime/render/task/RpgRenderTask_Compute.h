#pragma once

#include "core/RpgThreadPool.h"
#include "../RpgRenderResource.h"



class RpgRenderTask_Compute : public RpgThreadTask
{
public:
	ID3D12Fence* FenceSignal;
	uint64_t WaitFenceCopyValue;
	uint64_t FenceSignalValue;
	RpgRenderFrameContext FrameContext;


public:
	RpgRenderTask_Compute() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_Compute";
	}


	inline ID3D12CommandList* GetCommandList() const noexcept
	{
		return CmdListCompute.Get();
	}


private:
	ComPtr<ID3D12CommandAllocator> CmdAllocCompute;
	ComPtr<ID3D12GraphicsCommandList> CmdListCompute;

};
