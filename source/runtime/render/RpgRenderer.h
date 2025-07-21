#pragma once

#include "RpgRenderer2D.h"
#include "RpgRenderResource.h"
#include "task/RpgRenderTask_Copy.h"
#include "task/RpgRenderTask_Compute.h"



class RpgRenderer
{
	RPG_NOCOPY(RpgRenderer)

public:
	RpgRenderer(HWND in_WindowHandle, bool bEnableVsync) noexcept;
	~RpgRenderer() noexcept;

	void BeginRender(int frameIndex, float deltaTime) noexcept;
	void EndRender(int frameIndex, float deltaTime) noexcept;
	void Execute(int frameIndex, float deltaTime) noexcept;
	void RegisterWorld(const RpgWorld* world) noexcept;
	void UnregisterWorld(const RpgWorld* world) noexcept;


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

	inline void SetFinalTexture(int frameIndex, const RpgSharedTexture2D& texture) noexcept
	{
		FrameDatas[frameIndex].FinalTexture = texture;
	}

	inline void AddWorldShadowViewport(int frameIndex, const RpgWorld* world, RpgShadowViewport* viewport) noexcept
	{
		GetWorldContext(frameIndex, world).ShadowViewports.AddUnique(viewport);
	}

	inline void AddWorldSceneViewport(int frameIndex, const RpgWorld* world, RpgSceneViewport* viewport) noexcept
	{
		GetWorldContext(frameIndex, world).SceneViewports.AddUnique(viewport);
	}

	inline RpgWorldResource* GetWorldResource(int frameIndex, const RpgWorld* world) noexcept
	{
		return GetWorldContext(frameIndex, world).Resource.Get();
	}

	inline RpgRenderer2D& GetRenderer2D() noexcept
	{
		return Renderer2d;
	}


public:
	float Gamma;
	RpgRenderLight::EShadowQuality ShadowQuality;
	RpgRenderAntiAliasing::EMode AntiAliasingMode;


private:
	struct FWorldContext
	{
		const RpgWorld* World{ nullptr };
		RpgUniquePtr<RpgWorldResource> Resource;
		RpgArrayInline<RpgShadowViewport*, 16> ShadowViewports;
		RpgArrayInline<RpgSceneViewport*, 8> SceneViewports;


		inline bool operator==(const RpgWorld* rhs) const noexcept
		{
			return World == rhs;
		}

	};
	typedef RpgArray<FWorldContext, 4> FWorldContextArray;


private:
	void WaitFrameFinished(int frameIndex) noexcept;
	void SwapchainWaitAllPresents() noexcept;
	void SwapchainReleaseResources(bool bResize) noexcept;
	void SwapchainResize() noexcept;


	inline FWorldContext& GetWorldContext(int frameIndex, const RpgWorld* world) noexcept
	{
		FFrameData& frame = FrameDatas[frameIndex];

		const int index = frame.WorldContexts.FindIndexByCompare(world);
		RPG_Check(index != RPG_INDEX_INVALID);

		return frame.WorldContexts[index];
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

	RpgRenderer2D Renderer2d;


	struct FFrameData
	{
		ComPtr<ID3D12Fence> Fence;
		uint64_t FenceValue;

		RpgUniquePtr<RpgMaterialResource> MaterialResource;
		RpgUniquePtr<RpgMeshResource> MeshResource;
		RpgUniquePtr<RpgMeshSkinnedResource> MeshSkinnedResource;
		FWorldContextArray WorldContexts;
		RpgSharedTexture2D FinalTexture;

		RpgUniquePtr<RpgRenderTask_Copy> TaskCopy;
		RpgUniquePtr<RpgRenderTask_Compute> TaskCompute;

		ComPtr<ID3D12CommandAllocator> SwapChainCmdAlloc;
		ComPtr<ID3D12GraphicsCommandList> SwapChainCmdList;

		HANDLE PresentCompletedEvent;
	};
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];


#ifndef RPG_BUILD_SHIPPING
public:
	RpgVertexPrimitiveBatchLine* Debug_GetPrimitiveBatchLine(int frameIndex, const RpgWorld* world, bool bNoDepth) noexcept;
#endif // !RPG_BUILD_SHIPPING

};
