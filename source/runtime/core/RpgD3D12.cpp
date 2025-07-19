#include "RpgD3D12.h"
#include "core/RpgCommandLine.h"


extern "C" __declspec(dllexport) const UINT D3D12SDKVersion = 616;
extern "C" __declspec(dllexport) const char* D3D12SDKPath = ".\\D3D12\\";


RPG_LOG_DEFINE_CATEGORY(RpgLogD3D12, VERBOSITY_DEBUG)



RpgD3D12::FResourceDescriptorPool::FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE in_Type, uint32_t in_NumDescriptors, bool in_bShaderVisible) noexcept
    : Desc()
    , IncrementSize(0)
{
    Desc.NodeMask = 0;
    Desc.Type = in_Type;
    Desc.Flags = in_bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Desc.NumDescriptors = in_NumDescriptors;

    RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap)));

    IncrementSize = RpgD3D12::GetDevice()->GetDescriptorHandleIncrementSize(in_Type);
    DescriptorIndex = 0;
}



namespace RpgD3D12
{
	static ComPtr<IDXGIFactory6> Factory;
    static ComPtr<IDXGIAdapter3> Adapter;
	static ComPtr<ID3D12Device4> Device;
    static D3D_FEATURE_LEVEL MaxSupportedFeatureLevel;

    static ComPtr<ID3D12CommandQueue> CommandQueueDirect;
    static ComPtr<ID3D12CommandQueue> CommandQueueCompute;
    static ComPtr<ID3D12CommandQueue> CommandQueueCopy;

    static ComPtr<D3D12MA::Allocator> MemoryAllocator;
    static ComPtr<D3D12MA::Pool> MemoryPoolBufferCPU;
    static ComPtr<D3D12MA::Pool> MemoryPoolBufferGPU;
    static ComPtr<D3D12MA::Pool> MemoryPoolTextureGPU;
    static ComPtr<D3D12MA::Pool> MemoryPoolRenderTargetGPU;

    static bool bValidationLayer;
	static bool bInitialized;

    static int FrameIndex;

    struct FFrameData
    {
        FResourceDescriptorPool* DescriptorPool_RTV;
        FResourceDescriptorPool* DescriptorPool_DSV;
        FResourceDescriptorPool* DescriptorPool_CBV_SRV_UAV;
        FResourceDescriptorPool* DescriptorPool_TDI;
    };
    static FFrameData FrameDatas[RPG_FRAME_BUFFERING];


    static void* D3D12MA_Alloc(size_t size, size_t alignment, void*) noexcept
    {
        return RpgPlatformMemory::MemMallocAligned(size, alignment);
    }

    static void D3D12MA_Free(void* data, void*) noexcept
    {
        RpgPlatformMemory::MemFree(data);
    }

};



void RpgD3D12::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

    RPG_Log(RpgLogD3D12, "Initialize D3D12");

    UINT dxgiFactoryFlags = 0;
	bValidationLayer = RpgCommandLine::HasCommand("d3d12validation");

    if (bValidationLayer)
    {
        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

        ComPtr<ID3D12Debug1> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->SetEnableGPUBasedValidation(true);
            debugController->SetEnableSynchronizedCommandQueueValidation(true);
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
            {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
            };

            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = ARRAYSIZE(hide);
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }


    RPG_D3D12_Validate(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(Factory.ReleaseAndGetAddressOf())));


    ComPtr<IDXGIAdapter1> testAdapter;

    for (UINT adapterIndex = 0;
        SUCCEEDED(Factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(testAdapter.ReleaseAndGetAddressOf())));
        adapterIndex++)
    {
        DXGI_ADAPTER_DESC1 desc;
        RPG_D3D12_Validate(testAdapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(testAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            char adapterDescription[64];
            wcstombs(adapterDescription, desc.Description, 64);

            RPG_Log(RpgLogD3D12, "Found GPU with D3D12 support (Name: %s, VRAM: %u MiB, DriverVersion: xxx.xx)", 
                adapterDescription, 
                desc.DedicatedVideoMemory / RPG_MEMORY_SIZE_MiB(1)
            );

            RPG_D3D12_Validate(testAdapter->QueryInterface(IID_PPV_ARGS(&Adapter)));

            break;
        }
    }

    RPG_RuntimeErrorCheck(Adapter, "GPU with D3D12 support not found!");

    DXGI_QUERY_VIDEO_MEMORY_INFO queryVideoMemoryInfo{};
    RPG_D3D12_Validate(Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &queryVideoMemoryInfo));

    // Create the DX12 API device object.
    RPG_D3D12_Validate(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(Device.ReleaseAndGetAddressOf())));

    // Configure debug device (if active).
    if (bValidationLayer)
    {
        ComPtr<ID3D12InfoQueue> infoQueue;

        if (SUCCEEDED(Device.As(&infoQueue)))
        {
            //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

            D3D12_MESSAGE_ID hide[] =
            {
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                // Workarounds for debug layer issues on hybrid-graphics systems
                D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
                D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
            };

            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = ARRAYSIZE(hide);
            filter.DenyList.pIDList = hide;

            infoQueue->AddStorageFilterEntries(&filter);
        }
    }


    // Determine maximum supported feature level for this device
    {
        const D3D_FEATURE_LEVEL FEATURE_LEVELS[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS dataFeatureLevels =
        {
            static_cast<UINT>(ARRAYSIZE(FEATURE_LEVELS)), 
            FEATURE_LEVELS, 
            D3D_FEATURE_LEVEL_11_0
        };

        if (SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &dataFeatureLevels, sizeof(dataFeatureLevels))))
        {
            MaxSupportedFeatureLevel = dataFeatureLevels.MaxSupportedFeatureLevel;
        }
        else
        {
            MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        }
    }


    // Create command queues
    {
        D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
        cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        RPG_D3D12_Validate(Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&CommandQueueDirect)));
        RPG_D3D12_SetDebugName(CommandQueueDirect, "CmdQueue_Direct");

        cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        RPG_D3D12_Validate(Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&CommandQueueCompute)));
        RPG_D3D12_SetDebugName(CommandQueueCompute, "CmdQueue_Compute");

        cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        RPG_D3D12_Validate(Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&CommandQueueCopy)));
        RPG_D3D12_SetDebugName(CommandQueueCopy, "CmdQueue_Copy");
    }

    
    // Create memory pools
    {
        D3D12MA::ALLOCATION_CALLBACKS allocCallbacks{};
        allocCallbacks.pAllocate = D3D12MA_Alloc;
        allocCallbacks.pFree = D3D12MA_Free;

        D3D12MA::ALLOCATOR_DESC allocDesc{};
        allocDesc.pAdapter = Adapter.Get();
        allocDesc.pDevice = Device.Get();
        allocDesc.pAllocationCallbacks = &allocCallbacks;
        RPG_D3D12_Validate(D3D12MA::CreateAllocator(&allocDesc, &MemoryAllocator));

        D3D12MA::POOL_DESC poolDesc{};
        
        poolDesc.HeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        poolDesc.HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        RPG_D3D12_Validate(MemoryAllocator->CreatePool(&poolDesc, &MemoryPoolBufferCPU));
        poolDesc.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        RPG_D3D12_Validate(MemoryAllocator->CreatePool(&poolDesc, &MemoryPoolBufferGPU));

        poolDesc.HeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        poolDesc.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        RPG_D3D12_Validate(MemoryAllocator->CreatePool(&poolDesc, &MemoryPoolTextureGPU));

        poolDesc.HeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
        RPG_D3D12_Validate(MemoryAllocator->CreatePool(&poolDesc, &MemoryPoolRenderTargetGPU));
    }


    for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
    {
        FFrameData& frame = FrameDatas[f];

        frame.DescriptorPool_RTV = new FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 8, false);
        RPG_D3D12_SetDebugName(frame.DescriptorPool_RTV->GetHeap(), "%i_DescriptorHeap_RTV", f);

        frame.DescriptorPool_DSV = new FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32, false);
        RPG_D3D12_SetDebugName(frame.DescriptorPool_DSV->GetHeap(), "%i_DescriptorHeap_DSV", f);

        frame.DescriptorPool_CBV_SRV_UAV = new FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4, true);
        RPG_D3D12_SetDebugName(frame.DescriptorPool_CBV_SRV_UAV->GetHeap(), "%i_DescriptorHeap_CBV_SRV_UAV", f);

        frame.DescriptorPool_TDI = new FResourceDescriptorPool(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 512, true);
        RPG_D3D12_SetDebugName(frame.DescriptorPool_TDI->GetHeap(), "%i_DescriptorHeap_TDI", f);
    }


	bInitialized = true;
}


void RpgD3D12::Shutdown() noexcept
{
	if (!bInitialized)
	{
		return;
	}

    RPG_Log(RpgLogD3D12, "Shutdown D3D12");

    for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
    {
        FFrameData& frame = FrameDatas[f];
        delete frame.DescriptorPool_RTV;
        delete frame.DescriptorPool_DSV;
        delete frame.DescriptorPool_CBV_SRV_UAV;
        delete frame.DescriptorPool_TDI;
    }

    MemoryPoolRenderTargetGPU.Reset();
    MemoryPoolTextureGPU.Reset();
    MemoryPoolBufferGPU.Reset();
    MemoryPoolBufferCPU.Reset();
    MemoryAllocator.Reset();

    CommandQueueDirect.Reset();
    CommandQueueCompute.Reset();
    CommandQueueCopy.Reset();

    Device.Reset();
    Adapter.Reset();
    Factory.Reset();

#ifndef RPG_BUILD_SHIPPING
    ComPtr<IDXGIDebug1> dxgiDebug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
    {
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
#endif // !RPG_BUILD_SHIPPING

	bInitialized = false;
}


void RpgD3D12::BeginFrame(int frameIndex) noexcept
{
    FrameIndex = frameIndex;

    FFrameData& frame = FrameDatas[FrameIndex];
    frame.DescriptorPool_RTV->Reset();
    frame.DescriptorPool_DSV->Reset();
    frame.DescriptorPool_CBV_SRV_UAV->Reset();
    frame.DescriptorPool_TDI->Reset();
}


IDXGIFactory6* RpgD3D12::GetFactory() noexcept
{
    return Factory.Get();
}

IDXGIAdapter1* RpgD3D12::GetAdapter() noexcept
{
    return Adapter.Get();
}

ID3D12Device4* RpgD3D12::GetDevice() noexcept
{
    return Device.Get();
}

ID3D12CommandQueue* RpgD3D12::GetCommandQueueDirect() noexcept
{
    return CommandQueueDirect.Get();
}

ID3D12CommandQueue* RpgD3D12::GetCommandQueueCompute() noexcept
{
    return CommandQueueCompute.Get();
}

ID3D12CommandQueue* RpgD3D12::GetCommandQueueCopy() noexcept
{
    return CommandQueueCopy.Get();
}



ComPtr<D3D12MA::Allocation> RpgD3D12::CreateBuffer(size_t sizeBytes, bool bCpuAccess) noexcept
{
    RPG_Assert(sizeBytes > 0);

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = sizeBytes;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;

    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.CustomPool = bCpuAccess ? MemoryPoolBufferCPU.Get() : MemoryPoolBufferGPU.Get();

    ComPtr<D3D12MA::Allocation> allocation;
    RPG_D3D12_Validate(MemoryAllocator->CreateResource(&allocDesc, &bufferDesc, bCpuAccess ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &allocation, IID_NULL, nullptr));

    return allocation;
}


bool RpgD3D12::ResizeBuffer(ComPtr<D3D12MA::Allocation>& out_Buffer, size_t newSizeBytes, bool bCpuAccess) noexcept
{
    bool bShouldResize = (out_Buffer == nullptr);

    if (!bShouldResize)
    {
        const D3D12_RESOURCE_DESC desc = out_Buffer->GetResource()->GetDesc();
        bShouldResize = (desc.Width < newSizeBytes);
    }

    if (bShouldResize)
    {
        out_Buffer = CreateBuffer(newSizeBytes, bCpuAccess);
    }

    return bShouldResize;
}


ComPtr<D3D12MA::Allocation> RpgD3D12::CreateTexture2D(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, uint8_t mipLevel) noexcept
{
    RPG_Assert(width > 0 && height > 0 && mipLevel > 0);

    const D3D12_RESOURCE_DESC textureDesc = CreateResourceDesc_Texture(format, width, height, mipLevel);
    
    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.CustomPool = MemoryPoolTextureGPU.Get();

    ComPtr<D3D12MA::Allocation> allocation;
    RPG_D3D12_Validate(MemoryAllocator->CreateResource(&allocDesc, &textureDesc, initialState, nullptr, &allocation, IID_NULL, nullptr));

    return allocation;
}


ComPtr<D3D12MA::Allocation> RpgD3D12::CreateRenderTarget(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, RpgColorLinear clearColor) noexcept
{
    RPG_Assert(width > 0 && height > 0);

    D3D12_RESOURCE_DESC renderTargetDesc{};
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    renderTargetDesc.Format = format;
    renderTargetDesc.Alignment = 0;
    renderTargetDesc.Width = width;
    renderTargetDesc.Height = height;
    renderTargetDesc.DepthOrArraySize = 1;
    renderTargetDesc.MipLevels = 1;
    renderTargetDesc.SampleDesc.Count = 1;
    renderTargetDesc.SampleDesc.Quality = 0;

    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.CustomPool = MemoryPoolRenderTargetGPU.Get();

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = format;
    RpgPlatformMemory::MemCopy(clearValue.Color, &clearColor, sizeof(float) * 4);

    ComPtr<D3D12MA::Allocation> allocation;
    RPG_D3D12_Validate(MemoryAllocator->CreateResource(&allocDesc, &renderTargetDesc, initialState, &clearValue, &allocation, IID_NULL, nullptr));

    return allocation;
}


ComPtr<D3D12MA::Allocation> RpgD3D12::CreateDepthStencil(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height, float clearDepth, uint8_t clearStencil) noexcept
{
    RPG_Assert(width > 0 && height > 0);

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Format = format;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;

    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.CustomPool = MemoryPoolRenderTargetGPU.Get();

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = clearDepth;
    clearValue.DepthStencil.Stencil = clearStencil;

    ComPtr<D3D12MA::Allocation> allocation;
    RPG_D3D12_Validate(MemoryAllocator->CreateResource(&allocDesc, &depthStencilDesc, initialState, &clearValue, &allocation, IID_NULL, nullptr));

    return allocation;
}


ComPtr<D3D12MA::Allocation> RpgD3D12::CreateDepthCube(DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, uint16_t width, uint16_t height) noexcept
{
    D3D12_RESOURCE_DESC depthCubeDesc = CreateResourceDesc_Texture(format, width, height, 1, 6);
    depthCubeDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.CustomPool = MemoryPoolRenderTargetGPU.Get();

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    ComPtr<D3D12MA::Allocation> allocation;
    RPG_D3D12_Validate(MemoryAllocator->CreateResource(&allocDesc, &depthCubeDesc, initialState, &clearValue, &allocation, IID_NULL, nullptr));

    return allocation;
}


RpgD3D12::FResourceDescriptor RpgD3D12::AllocateDescriptor_RTV(ID3D12Resource* renderTargetResource) noexcept
{
    const FResourceDescriptor descriptor = FrameDatas[FrameIndex].DescriptorPool_RTV->AllocateDescriptor();
    RpgD3D12::GetDevice()->CreateRenderTargetView(renderTargetResource, nullptr, descriptor.CpuHandle);

    return descriptor;
}


RpgD3D12::FResourceDescriptor RpgD3D12::AllocateDescriptor_DSV(ID3D12Resource* depthStencilResource) noexcept
{
    const FResourceDescriptor descriptor = FrameDatas[FrameIndex].DescriptorPool_DSV->AllocateDescriptor();
    RpgD3D12::GetDevice()->CreateDepthStencilView(depthStencilResource, nullptr, descriptor.CpuHandle);

    return descriptor;
}


RpgD3D12::FResourceDescriptor RpgD3D12::AllocateDescriptor_TDI(ID3D12Resource* textureResource, DXGI_FORMAT format) noexcept
{
    const FResourceDescriptor descriptor = FrameDatas[FrameIndex].DescriptorPool_TDI->AllocateDescriptor();

    const D3D12_RESOURCE_DESC resDesc = textureResource->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Format = (format == DXGI_FORMAT_UNKNOWN) ? resDesc.Format : format;
    viewDesc.Texture2D.MostDetailedMip = 0;
    viewDesc.Texture2D.MipLevels = -1;
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    RpgD3D12::GetDevice()->CreateShaderResourceView(textureResource, &viewDesc, descriptor.CpuHandle);

    return descriptor;
}


RpgD3D12::FResourceDescriptor RpgD3D12::AllocateDescriptor_TDI_Cube(ID3D12Resource* textureResource, DXGI_FORMAT format) noexcept
{
    const FResourceDescriptor descriptor = FrameDatas[FrameIndex].DescriptorPool_TDI->AllocateDescriptor();

    const D3D12_RESOURCE_DESC resDesc = textureResource->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    viewDesc.Format = format;
    viewDesc.Texture2D.MostDetailedMip = 0;
    viewDesc.Texture2D.MipLevels = -1;
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    RpgD3D12::GetDevice()->CreateShaderResourceView(textureResource, &viewDesc, descriptor.CpuHandle);

    return descriptor;
}


ID3D12DescriptorHeap* RpgD3D12::GetDescriptorHeap_TDI() noexcept
{
    return FrameDatas[FrameIndex].DescriptorPool_TDI->GetHeap();
}
