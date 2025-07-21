#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#ifndef RPG_BUILD_SHIPPING
#include <dxgidebug.h>
#endif // RPG_BUILD_SHIPPING

#include <wrl/client.h>

using namespace Microsoft::WRL;

#include "core/RpgPlatform.h"

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "thirdparty/D3D12MA/D3D12MemAlloc.h"


#define RPG_D3D12_Validate(hr)		RPG_Validate((hr) == S_OK)


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogD3D12)


#ifndef RPG_BUILD_SHIPPING

#define RPG_D3D12_SetDebugName(x, format, ...)			\
if (x)													\
{														\
	char __cDebugName[64];								\
	snprintf(__cDebugName, 64, format, __VA_ARGS__);	\
	wchar_t __wDebugName[64];							\
	mbstowcs(__wDebugName, __cDebugName, 64);			\
	RPG_D3D12_Validate(x->SetName(__wDebugName));		\
}

#define RPG_D3D12_SetDebugNameAllocation(x, format, ...)			\
if (x)																\
{																	\
	char __cDebugName[64];											\
	snprintf(__cDebugName, 64, format, __VA_ARGS__);				\
	wchar_t __wDebugName[64];										\
	mbstowcs(__wDebugName, __cDebugName, 64);						\
	x->SetName(__wDebugName);										\
	RPG_D3D12_Validate(x->GetResource()->SetName(__wDebugName));	\
}

#else
#define RPG_D3D12_SetDebugName(x, format, ...)
#define RPG_D3D12_SetDebugNameAllocation(x, format, ...)

#endif // !RPG_BUILD_SHIPPING



namespace RpgD3D12
{
	struct FResourceDescriptor
	{
	public:
		int Index;
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;


	public:
		FResourceDescriptor() noexcept
			: Index(RPG_INDEX_INVALID)
			, CpuHandle()
			, GpuHandle()
		{
		}

		[[nodiscard]] inline bool IsValid() const noexcept
		{
			return Index != RPG_INDEX_INVALID && CpuHandle.ptr;
		}

	};



	class FResourceDescriptorPool
	{
	public:
		FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE in_Type, uint32_t in_NumDescriptors, bool in_bShaderVisible) noexcept;

		inline void Reset() noexcept
		{
			RPG_IsMainThread();
			DescriptorIndex = 0;
		}


		inline ID3D12DescriptorHeap* GetHeap() const noexcept
		{
			return Heap.Get();
		}


		[[nodiscard]] inline FResourceDescriptor AllocateDescriptor() noexcept
		{
			FResourceDescriptor descriptor{};

			int expected = 0;
			int desired = 0;

			do
			{
				expected = DescriptorIndex;
				desired = expected + 1;
			}
			while (InterlockedCompareExchange(&DescriptorIndex, desired, expected) != expected);

			descriptor.Index = expected;
			RPG_CheckV(descriptor.Index < static_cast<int>(Desc.NumDescriptors), "Exceeds maximum descriptor capacity!");

			descriptor.CpuHandle.ptr = Heap->GetCPUDescriptorHandleForHeapStart().ptr + static_cast<SIZE_T>(descriptor.Index * IncrementSize);

			if (Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			{
				descriptor.GpuHandle.ptr = Heap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<UINT64>(descriptor.Index * IncrementSize);
			}

			return descriptor;
		}


	private:
		ComPtr<ID3D12DescriptorHeap> Heap;
		D3D12_DESCRIPTOR_HEAP_DESC Desc;
		uint32_t IncrementSize;
		RpgAtomicInt DescriptorIndex;

	};



	extern void Initialize() noexcept;
	extern void Shutdown() noexcept;

	extern void BeginFrame(int frameIndex) noexcept;

	extern IDXGIFactory6* GetFactory() noexcept;
	extern IDXGIAdapter1* GetAdapter() noexcept;
	extern ID3D12Device4* GetDevice() noexcept;
	extern ID3D12CommandQueue* GetCommandQueueDirect() noexcept;
	extern ID3D12CommandQueue* GetCommandQueueCompute() noexcept;
	extern ID3D12CommandQueue* GetCommandQueueCopy() noexcept;

	[[nodiscard]] extern ComPtr<D3D12MA::Allocation> CreateBuffer(size_t sizeBytes, bool bCpuAccess) noexcept;
	extern bool ResizeBuffer(ComPtr<D3D12MA::Allocation>& out_Buffer, size_t newSizeBytes, bool bCpuAccess) noexcept;
	[[nodiscard]] extern ComPtr<D3D12MA::Allocation> CreateTexture2D(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, uint8_t mipLevel) noexcept;
	[[nodiscard]] extern ComPtr<D3D12MA::Allocation> CreateRenderTarget(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, RpgColorLinear clearColor) noexcept;
	[[nodiscard]] extern ComPtr<D3D12MA::Allocation> CreateDepthStencil(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, float clearDepth, uint8_t clearStencil) noexcept;
	[[nodiscard]] extern ComPtr<D3D12MA::Allocation> CreateDepthCube(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height) noexcept;

	
	template<typename TData = void>
	inline TData* MapBuffer(D3D12MA::Allocation* buffer) noexcept
	{
		RPG_Check(buffer);
		void* map = nullptr;
		RPG_D3D12_Validate(buffer->GetResource()->Map(0, nullptr, &map));

		return reinterpret_cast<TData*>(map);
	}

	inline void UnmapBuffer(D3D12MA::Allocation* buffer) noexcept
	{
		buffer->GetResource()->Unmap(0, nullptr);
	}


	[[nodiscard]] extern FResourceDescriptor AllocateDescriptor_RTV(int frameIndex, ID3D12Resource* renderTargetResource) noexcept;
	[[nodiscard]] extern FResourceDescriptor AllocateDescriptor_DSV(int frameIndex, ID3D12Resource* depthStencilResource) noexcept;
	[[nodiscard]] extern FResourceDescriptor AllocateDescriptor_TDI(int frameIndex, ID3D12Resource* textureResource, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN) noexcept;
	[[nodiscard]] extern FResourceDescriptor AllocateDescriptor_TDI_Cube(int frameIndex, ID3D12Resource* textureResource, DXGI_FORMAT format) noexcept;
	[[nodiscard]] extern ID3D12DescriptorHeap* GetDescriptorHeap_TDI(int frameIndex) noexcept;


	static inline D3D12_RESOURCE_DESC CreateResourceDesc_Texture(DXGI_FORMAT format, uint16_t width, uint16_t height, uint8_t mipLevel, uint16_t arraySize = 1) noexcept
	{
		D3D12_RESOURCE_DESC textureDesc{};
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDesc.Format = format;
		textureDesc.Alignment = 0;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.DepthOrArraySize = arraySize;
		textureDesc.MipLevels = mipLevel;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;

		return textureDesc;
	}


	static inline D3D12_RESOURCE_BARRIER CreateResourceBarrier_Transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		return barrier;
	}

}; // RpgD3D12




#define RPG_D3D12_COMMAND_Begin(cmdAlloc, cmdList)					\
{																	\
	RPG_D3D12_Validate(cmdAlloc.Get()->Reset());					\
	RPG_D3D12_Validate(cmdList->Reset(cmdAlloc.Get(), nullptr));	\
}


#define RPG_D3D12_COMMAND_End(cmdList) RPG_D3D12_Validate(cmdList->Close())



namespace RpgD3D12Command
{
	inline void SetViewport(ID3D12GraphicsCommandList* cmdList, int x, int y, int w, int h, float minDepth, float maxDepth) noexcept
	{
		RPG_Assert(cmdList);

		D3D12_VIEWPORT viewport{};
		viewport.TopLeftX = static_cast<FLOAT>(x);
		viewport.TopLeftY = static_cast<FLOAT>(y);
		viewport.Width = static_cast<FLOAT>(w);
		viewport.Height = static_cast<FLOAT>(h);
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;

		cmdList->RSSetViewports(1, &viewport);
	}


	inline void SetScissor(ID3D12GraphicsCommandList* cmdList, int left, int top, int right, int bottom) noexcept
	{
		RPG_Assert(cmdList);

		D3D12_RECT scissor{};
		scissor.left = static_cast<LONG>(left);
		scissor.top = static_cast<LONG>(top);
		scissor.right = static_cast<LONG>(right);
		scissor.bottom = static_cast<LONG>(bottom);

		cmdList->RSSetScissorRects(1, &scissor);
	}


	inline void TransitionAllSubresources(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		RPG_Assert(cmdList);
		RPG_Assert(resource);

		if (stateBefore == stateAfter)
		{
			return;
		}

		const D3D12_RESOURCE_BARRIER barrier = RpgD3D12::CreateResourceBarrier_Transition(resource, stateBefore, stateAfter);
		cmdList->ResourceBarrier(1, &barrier);
	}


	inline void SetAndClearRenderTargets(ID3D12GraphicsCommandList* cmdList, const RpgD3D12::FResourceDescriptor* renderTargetDescriptors, int renderTargetCount, RpgColorLinear clearColor, const RpgD3D12::FResourceDescriptor* depthStencilDescriptor, float clearDepth, uint8_t clearStencil) noexcept
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuRenderTargetHandles[8]{};

		for (int i = 0; i < renderTargetCount; ++i)
		{
			cpuRenderTargetHandles[i] = renderTargetDescriptors[i].CpuHandle;
		}

		cmdList->OMSetRenderTargets(renderTargetCount, cpuRenderTargetHandles, false,
			depthStencilDescriptor ? &depthStencilDescriptor->CpuHandle : nullptr
		);

		for (int i = 0; i < renderTargetCount; ++i)
		{
			cmdList->ClearRenderTargetView(cpuRenderTargetHandles[i], (const float*)&clearColor, 0, nullptr);
		}

		if (depthStencilDescriptor)
		{
			cmdList->ClearDepthStencilView(depthStencilDescriptor->CpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearDepth, clearStencil, 0, nullptr);
		}
	}

};
