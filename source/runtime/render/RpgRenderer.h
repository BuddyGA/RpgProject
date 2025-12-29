#pragma once

#include "RpgRenderResource.h"
#include "task/RpgRenderTask_Copy.h"
#include "task/RpgRenderTask_Compute.h"


class RpgSceneViewport;



extern class RpgRenderer* g_Renderer;

class RpgRenderer
{
	RPG_NOCOPYMOVE(RpgRenderer)

public:
	float Gamma;
	RpgRenderLight::EShadowQuality ShadowQuality;
	RpgRenderAntiAliasing::EMode AntiAliasingMode;


public:
	RpgRenderer(HWND in_WindowHandle, bool bEnableVsync) noexcept;
	~RpgRenderer() noexcept;

	void BeginPreRender(int frameIndex, float deltaTime) noexcept;
	void EndPreRender(int frameIndex, float deltaTime) noexcept;
	void Render(int frameIndex, float deltaTime) noexcept;


	inline void SetVsync(bool bEnabled) noexcept
	{
		if (bVsync == bEnabled)
		{
			return;
		}

		bPendingChangeVsync = bEnabled;
	}

	inline bool GetVsync() const noexcept
	{
		return bVsync;
	}

	inline RpgPointInt GetSwapChainDimension() const noexcept
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		SwapChain->GetDesc1(&desc);

		return RpgPointInt(static_cast<int>(desc.Width), static_cast<int>(desc.Height));
	}


	inline void AddSceneViewport(RpgSceneViewport* viewport) noexcept
	{
		FrameDatas[FrameIndex].SceneViewports.AddUnique(viewport);
	}

	inline void SetFinalTexture(const RpgSharedTexture2D& texture) noexcept
	{
		FrameDatas[FrameIndex].FinalTexture = texture;
	}


private:
	HWND WindowHandle;

	uint8_t bSupportTearing : 1;
	uint8_t bSupportHDR : 1;
	uint8_t bPendingChangeVsync : 1;
	uint8_t bVsync : 1;

	ComPtr<IDXGISwapChain4> SwapChain;

	DXGI_FORMAT BackbufferFormat;
	ComPtr<ID3D12Resource> BackbufferResources[RPG_FRAME_BUFFERING];
	uint32_t BackbufferIndex;

	struct FFrameData
	{
		ComPtr<ID3D12Fence> Fence;
		uint64_t FenceValue;

		RpgRenderFrameContext Context;
		RpgArrayInline<RpgSceneViewport*, 8> SceneViewports;
		RpgSharedTexture2D FinalTexture;

		RpgRenderTask_Copy TaskCopy;
		RpgRenderTask_Compute TaskCompute;

		ComPtr<ID3D12CommandAllocator> SwapChainCmdAlloc;
		ComPtr<ID3D12GraphicsCommandList> SwapChainCmdList;

		HANDLE PresentCompletedEvent;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];
	int FrameIndex;


private:
	void WaitFrameFinished(int frameIndex) noexcept;
	void SwapchainWaitAllPresents() noexcept;
	void SwapchainReleaseResources(bool bResize) noexcept;
	void SwapchainResize() noexcept;

};
