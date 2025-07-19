#include "RpgRenderTask_RenderPass.h"



RpgRenderTask_RenderPass::RpgRenderTask_RenderPass() noexcept
{
	RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdAllocDirect)));
	RPG_D3D12_SetDebugName(CmdAllocDirect, "CmdAllocDirect_AsyncTaskRenderPass");

	RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&CmdListDirect)));
	RPG_D3D12_SetDebugName(CmdListDirect, "CmdListDirect_AsyncTaskRenderPass");

	WorldResource = nullptr;
}


void RpgRenderTask_RenderPass::Reset() noexcept
{
	RpgThreadTask::Reset();

	WorldResource = nullptr;
}


void RpgRenderTask_RenderPass::Execute() noexcept
{
	RPG_Assert(WorldResource);

	RPG_D3D12_COMMAND_Begin(CmdAllocDirect, CmdListDirect);
	CommandDraw(CmdListDirect.Get());
	RPG_D3D12_COMMAND_End(CmdListDirect);
}
