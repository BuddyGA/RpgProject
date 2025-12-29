#include "RpgRenderer.h"
#include "RpgRenderPipeline.h"
#include "RpgSceneViewport.h"
#include "task/RpgRenderTask_Capture.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogRenderer, VERBOSITY_DEBUG)


RpgRenderer* g_Renderer = nullptr;


RpgRenderer::RpgRenderer(HWND in_WindowHandle, bool bEnableVsync) noexcept
	: FrameDatas()
	, FrameIndex(0)
{
	Gamma = 1.25f;
	ShadowQuality = RpgRenderLight::SHADOW_QUALITY_MEDIUM;
	AntiAliasingMode = RpgRenderAntiAliasing::MODE_NONE;

	WindowHandle = in_WindowHandle;

	IDXGIFactory6* factory = RpgD3D12::GetFactory();
	BOOL bCheckTearingSupport = FALSE;

	if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bCheckTearingSupport, sizeof(BOOL))))
	{
		bSupportTearing = 1;
	}

	bSupportHDR = 0;
	bPendingChangeVsync = bEnableVsync;
	bVsync = 0;

	BackbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FFrameData& frame = FrameDatas[f];
		RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frame.Fence)));
		frame.FenceValue = 0;
		RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.SwapChainCmdAlloc)));
		RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&frame.SwapChainCmdList)));
	}
}


RpgRenderer::~RpgRenderer() noexcept
{
	RPG_LogDebug(RpgLogRenderer, "Destruct renderer");

	SwapchainWaitAllPresents();
	SwapchainReleaseResources(false);

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		WaitFrameFinished(f);
	}
}


void RpgRenderer::BeginPreRender(int frameIndex, float deltaTime) noexcept
{
	FrameIndex = frameIndex;

	RpgD3D12::BeginFrame(FrameIndex);

	WaitFrameFinished(FrameIndex);

	FFrameData& frame = FrameDatas[FrameIndex];

	frame.Context.Index = frameIndex;
	frame.Context.DeltaTime = deltaTime;
	frame.Context.ShadowQuality = ShadowQuality;
	frame.Context.AntiAliasingMode = AntiAliasingMode;
	frame.Context.ResetResources();

	frame.SceneViewports.Clear();
	frame.FinalTexture.Release();

	SwapchainResize();
}


void RpgRenderer::EndPreRender(int frameIndex, float deltaTime) noexcept
{
	RpgArrayInline<RpgThreadTask*, 8> preRenderTasks;

	FFrameData& frame = FrameDatas[frameIndex];

	RpgArrayInline<RpgRenderTask_Capture, 8> captureTasks;

	for (RpgSceneViewport* viewport : frame.SceneViewports)
	{
		viewport->UpdateFrame(frame.Context);

		RpgRenderTask_Capture& task = captureTasks.Add();
		task.SceneViewport = viewport;

		preRenderTasks.AddValue(&task);
	}

	RpgThreadPool::SubmitOrExecuteTasks<RPG_RENDER_ASYNC_TASK>(preRenderTasks.GetData(), preRenderTasks.GetCount());
	RPG_THREAD_TASK_WaitAll(preRenderTasks.GetData(), preRenderTasks.GetCount());
}


void RpgRenderer::Render(int frameIndex, float deltaTime) noexcept
{
	FFrameData& frame = FrameDatas[frameIndex];

	// Default material fullscreen
	RpgRenderResource_Material::FMaterialID fullscreenMaterialResourceId;
	{
		RpgSharedMaterial defMatFullscreen = RpgMaterial::GetDefault(RpgMaterialDefault::POSTPROCESS_FULLSCREEN);

		if (frame.FinalTexture)
		{
			defMatFullscreen->SetParameterTextureValue(RpgMaterialParameterTexture::BASE_COLOR, frame.FinalTexture);
		}

		defMatFullscreen->SetParameterScalarValue("gamma", Gamma);
		fullscreenMaterialResourceId = frame.Context.ResourceMaterial.AddMaterial(defMatFullscreen);
	}

	frame.Context.UpdateResources();

	// async copy
	RpgRenderTask_Copy* taskCopy = &frame.TaskCopy;
	{
		taskCopy->Reset();
		taskCopy->FenceSignal = frame.Fence.Get();
		taskCopy->FenceSignalValue = ++frame.FenceValue;
		taskCopy->FrameContext = &frame.Context;
		RpgThreadPool::SubmitOrExecuteTasks<RPG_RENDER_ASYNC_TASK>((RpgThreadTask**)&taskCopy, 1);
	}

	// async compute
	RpgRenderTask_Compute* taskCompute = &frame.TaskCompute;
	{
		taskCompute->Reset();
		taskCompute->FenceSignal = frame.Fence.Get();
		taskCompute->WaitFenceCopyValue = frame.FenceValue;
		taskCompute->FenceSignalValue = ++frame.FenceValue;
		taskCompute->FrameContext = &frame.Context;
		RpgThreadPool::SubmitOrExecuteTasks<RPG_RENDER_ASYNC_TASK>((RpgThreadTask**)&taskCompute, 1);
	}

	// async task forward pass
	RpgRenderTask_RenderPass_Forward_Array taskForwardPasses;
	{
		for (RpgSceneViewport* viewport : frame.SceneViewports)
		{
			viewport->SetupRenderPasses(frame.Context, taskForwardPasses);
		}

		RpgThreadPool::SubmitOrExecuteTasks<RPG_RENDER_ASYNC_TASK>(reinterpret_cast<RpgThreadTask**>(taskForwardPasses.GetData()), taskForwardPasses.GetCount());
	}

	// Begin swapchain rendering
	const RpgPointInt swapchainDimension = GetSwapChainDimension();
	ID3D12Resource* swapchainBackbuffer = BackbufferResources[BackbufferIndex].Get();
	RpgD3D12::FResourceDescriptor backbufferDescriptor = RpgD3D12::AllocateDescriptor_RTV(frameIndex, swapchainBackbuffer);

	ID3D12GraphicsCommandList* cmdList = frame.SwapChainCmdList.Get();
	RPG_D3D12_COMMAND_Begin(frame.SwapChainCmdAlloc, cmdList);
	{
		// Set viewport
		RpgD3D12Command::SetViewport(cmdList, 0, 0, swapchainDimension.X, swapchainDimension.Y, 0.0f, 1.0f);

		// Set scissor
		RpgD3D12Command::SetScissor(cmdList, 0, 0, swapchainDimension.X, swapchainDimension.Y);

		// Transition backbuffer resource to render target
		RpgD3D12Command::TransitionAllSubresources(cmdList, swapchainBackbuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Set and clear render target
		RpgD3D12Command::SetAndClearRenderTargets(cmdList, &backbufferDescriptor, 1, RpgColorLinear(0.1f, 0.15f, 0.2f), nullptr, 1.0f, 0);

		// Set root signature and global texture descriptor table (dynamic indexing)
		cmdList->SetGraphicsRootSignature(RpgRenderPipeline::GetRootSignatureGraphics());

		// Set descriptor table (texture dynamic indexing)
		ID3D12DescriptorHeap* textureDescriptorHeap = RpgD3D12::GetDescriptorHeap_TDI(frameIndex);
		cmdList->SetDescriptorHeaps(1, &textureDescriptorHeap);
		cmdList->SetGraphicsRootDescriptorTable(RpgRenderPipeline::GRPI_TEXTURES, textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		// Bind shader resource material
		frame.Context.ResourceMaterial.CommandBindShaderResources(cmdList);

		// Set topology triangle-list
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw fullscreen final texture
		if (frame.FinalTexture)
		{
			if (frame.FinalTexture->IsRenderTarget() || frame.FinalTexture->IsDepthStencil())
			{
				ID3D12Resource* textureResource = frame.FinalTexture->GPU_GetResource();

				const D3D12_RESOURCE_STATES beforeState = frame.FinalTexture->GPU_GetState();

				RpgD3D12Command::TransitionAllSubresources(cmdList, textureResource,
					frame.FinalTexture->IsRenderTarget() ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_DEPTH_WRITE,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
				);
			}

			frame.Context.ResourceMaterial.CommandBindMaterial(cmdList, fullscreenMaterialResourceId);

			cmdList->DrawInstanced(3, 1, 0, 0);
		}

		// Transition backbuffer resource to present
		RpgD3D12Command::TransitionAllSubresources(cmdList, swapchainBackbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	RPG_D3D12_COMMAND_End(cmdList);


	ID3D12CommandQueue* cmdQueueDirect = RpgD3D12::GetCommandQueueDirect();
	RpgArrayInline<ID3D12CommandList*, 8> directCommandLists;


	// Wait all shadow pass tasks
	/*
	RPG_THREAD_TASK_WaitAll(taskShadowPasses.GetData(), taskShadowPasses.GetCount());
	{
		RpgArrayInline<ID3D12CommandList*, 32> directCommandLists;
		for (int i = 0; i < taskShadowPasses.GetCount(); ++i)
		{
			directCommandLists.AddValue(taskShadowPasses[i]->GetCommandList());
		}

		if (!directCommandLists.IsEmpty())
		{
			RPG_D3D12_Validate(cmdQueueDirect->Wait(frame.Fence.Get(), frame.FenceValue++));
			cmdQueueDirect->ExecuteCommandLists(directCommandLists.GetCount(), directCommandLists.GetData());
			RPG_D3D12_Validate(cmdQueueDirect->Signal(frame.Fence.Get(), ++frame.FenceValue));
		}
	}
	*/

	// Wait all forward pass tasks
	/*
	RPG_THREAD_TASK_WaitAll(taskForwardPasses.GetData(), taskForwardPasses.GetCount());

	for (int i = 0; i < taskForwardPasses.GetCount(); ++i)
	{
		directCommandLists.AddValue(taskForwardPasses[i]->GetCommandList());
	}
	*/

	directCommandLists.AddValue(cmdList);

	// Execute/submit recorded direct command lists
	if (!directCommandLists.IsEmpty())
	{
		if (frame.FenceValue > 0)
		{
			RPG_D3D12_Validate(cmdQueueDirect->Wait(frame.Fence.Get(), frame.FenceValue));
		}

		cmdQueueDirect->ExecuteCommandLists(directCommandLists.GetCount(), directCommandLists.GetData());

		// Present
		if (frame.PresentCompletedEvent == NULL)
		{
			frame.PresentCompletedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		HRESULT hr = SwapChain->Present(bVsync, (bSupportTearing && !bVsync) ? DXGI_PRESENT_ALLOW_TEARING : 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			RPG_LogError(RpgLogD3D12, "Present failed: DXGI_ERROR_DEVICE_REMOVED!");
		}
		else if (hr == DXGI_ERROR_DEVICE_RESET)
		{
			RPG_LogError(RpgLogD3D12, "Present failed: DXGI_ERROR_DEVICE_RESET!");
		}
		else if (hr != S_OK)
		{
			RPG_LogWarn(RpgLogD3D12, "Present failed!");
		}

		BackbufferIndex = SwapChain->GetCurrentBackBufferIndex();

		RPG_D3D12_Validate(cmdQueueDirect->Signal(frame.Fence.Get(), ++frame.FenceValue));
	}

	if (frame.FinalTexture && (frame.FinalTexture->IsRenderTarget() || frame.FinalTexture->IsDepthStencil()))
	{
		frame.FinalTexture->GPU_SetState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	taskCopy->Wait();
	taskCompute->Wait();
}


void RpgRenderer::WaitFrameFinished(int frameIndex) noexcept
{
	FFrameData& frame = FrameDatas[frameIndex];

	const uint64_t fenceCompletedValue = frame.Fence->GetCompletedValue();

	if (fenceCompletedValue < frame.FenceValue)
	{
		HANDLE waitFenceCompletedHandle = CreateEventA(NULL, FALSE, FALSE, NULL);
		frame.Fence->SetEventOnCompletion(frame.FenceValue, waitFenceCompletedHandle);
		WaitForSingleObject(waitFenceCompletedHandle, INFINITE);
		CloseHandle(waitFenceCompletedHandle);
	}
}


void RpgRenderer::SwapchainWaitAllPresents() noexcept
{
	HANDLE waitCompletedEvents[RPG_FRAME_BUFFERING];
	int waitCount = 0;

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FFrameData& frame = FrameDatas[f];

		if (frame.Fence->GetCompletedValue() < frame.FenceValue)
		{
			frame.Fence->SetEventOnCompletion(frame.FenceValue, frame.PresentCompletedEvent);
			waitCompletedEvents[waitCount++] = frame.PresentCompletedEvent;
		}
	}

	if (waitCount > 0)
	{
		WaitForMultipleObjects(waitCount, waitCompletedEvents, TRUE, INFINITE);
	}
}


void RpgRenderer::SwapchainReleaseResources(bool bResize) noexcept
{
	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		BackbufferResources[f].Reset();
	}

	if (!bResize)
	{
		SwapChain.Reset();
	}
}


void RpgRenderer::SwapchainResize() noexcept
{
	RECT windowRect;
	GetClientRect(WindowHandle, &windowRect);
	RpgPointInt windowDimension(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);

	// Maybe minimized
	if (windowDimension.X == 0 && windowDimension.Y == 0)
	{
		return;
	}

	DXGI_SWAP_CHAIN_DESC1 desc{};

	if (SwapChain)
	{
		SwapChain->GetDesc1(&desc);
	}

	bool bShouldResize = (SwapChain == nullptr);

	if (static_cast<UINT>(windowDimension.X) != desc.Width || static_cast<UINT>(windowDimension.Y) != desc.Height)
	{
		RPG_LogDebug(RpgLogD3D12, "Resizing swapchain... Adjusting to window size (Swapchain: %i, %i) (Window: %i, %i)!", desc.Width, desc.Height, windowDimension.X, windowDimension.Y);
		bShouldResize = true;
	}

	if (bVsync != bPendingChangeVsync)
	{
		RPG_LogDebug(RpgLogD3D12, "Resizing swapchain... Vsync changed!");
		bShouldResize = true;
	}

	if (!bShouldResize)
	{
		return;
	}

	bVsync = bPendingChangeVsync;
	const UINT flags = bSupportTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	if (SwapChain == nullptr)
	{
		RPG_LogDebug(RpgLogD3D12, "Create swapchain");

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.Flags = flags;
		swapchainDesc.BufferCount = RPG_FRAME_BUFFERING;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.Format = BackbufferFormat;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{};
		fullscreenDesc.Windowed = TRUE;

		IDXGIFactory6* factory = RpgD3D12::GetFactory();

		ComPtr<IDXGISwapChain1> tempSwapchain;
		RPG_D3D12_Validate(factory->CreateSwapChainForHwnd(RpgD3D12::GetCommandQueueDirect(), WindowHandle, &swapchainDesc, &fullscreenDesc, nullptr, &tempSwapchain));
		RPG_D3D12_Validate(factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER));
		RPG_D3D12_Validate(tempSwapchain->QueryInterface(IID_PPV_ARGS(&SwapChain)));
	}
	else
	{
		SwapchainWaitAllPresents();
		SwapchainReleaseResources(true);

		RPG_LogDebug(RpgLogD3D12, "Resize swapchain %i, %i", windowDimension.X, windowDimension.Y);
		RPG_D3D12_Validate(SwapChain->ResizeBuffers(RPG_FRAME_BUFFERING, static_cast<UINT>(windowDimension.X), static_cast<UINT>(windowDimension.Y), BackbufferFormat, flags));
	}

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		RPG_D3D12_Validate(SwapChain->GetBuffer(f, IID_PPV_ARGS(&BackbufferResources[f])));
		RPG_D3D12_SetDebugName(BackbufferResources[f], "_%i_RES_SwapBackbuffer", f);
	}

	RPG_LogDebug(RpgLogD3D12, "Swapchain resized successfully");

	BackbufferIndex = SwapChain->GetCurrentBackBufferIndex();
}
