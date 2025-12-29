#include "RpgRenderTask_Copy.h"
#include "../RpgRenderResource.h"
#include "../RpgRenderer2D.h"



RpgRenderTask_Copy::RpgRenderTask_Copy() noexcept
{
	FenceSignal = nullptr;
	FenceSignalValue = 0;
	Renderer2d = nullptr;

	RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&CmdAllocCopy)));
	RPG_D3D12_SetDebugName(CmdAllocCopy, "CmdAllocCopy_AsyncTaskCopy");

	RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&CmdListCopy)));
	RPG_D3D12_SetDebugName(CmdListCopy, "CmdListCopy_AsyncTaskCopy");
}


void RpgRenderTask_Copy::Reset() noexcept
{
	RpgThreadTask::Reset();

	FenceSignal = nullptr;
	FenceSignalValue = 0;
	Renderer2d = nullptr;
	WorldResources.Clear();
}


void RpgRenderTask_Copy::Execute() noexcept
{
	RPG_Assert(FenceSignal);

	ID3D12GraphicsCommandList* cmdList = CmdListCopy.Get();
	RPG_D3D12_COMMAND_Begin(CmdAllocCopy, cmdList);

	FrameContext.MaterialResource->CommandCopy(cmdList);
	FrameContext.MeshResource->CommandCopy(cmdList);
	FrameContext.MeshSkinnedResource->CommandCopy(cmdList);

	Renderer2d->CommandCopy(FrameContext, cmdList);

	for (int i = 0; i < WorldResources.GetCount(); ++i)
	{
		WorldResources[i]->CommandCopy(cmdList);
	}

	RPG_D3D12_COMMAND_End(cmdList);

	// Execute/submit command list
	ID3D12CommandQueue* cmdQueueCopy = RpgD3D12::GetCommandQueueCopy();
	cmdQueueCopy->ExecuteCommandLists(1, (ID3D12CommandList**)&cmdList);
	RPG_D3D12_Validate(cmdQueueCopy->Signal(FenceSignal, FenceSignalValue));

}
