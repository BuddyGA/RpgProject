#pragma once

#include "core/RpgThreadPool.h"
#include "../RpgMaterial.h"



class RpgRenderTask_CompilePSO : public RpgThreadTask
{
public:
	ID3D12RootSignature* RootSignature;
	RpgName Name;
	RpgRenderPipelineState PipelineState;


public:
	RpgRenderTask_CompilePSO() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;

	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_CompilePSO";
	}


	[[nodiscard]] inline ComPtr<ID3D12PipelineState> GetCompiledPSO() noexcept
	{
		return std::move(PSO);
	}


private:
	ComPtr<ID3D12PipelineState> PSO;

};
