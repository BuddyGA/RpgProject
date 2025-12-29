#include "RpgRenderThread.h"
#include "RpgRenderer.h"



namespace RpgRenderThread
{
	static bool bRunning;
	static HANDLE Handle;
	static HANDLE AwakeSemaphore;


	struct FFrameData
	{
		HANDLE DoneSemaphore;
		RpgRenderer* Renderer;
		float DeltaTime;
	};
	static FFrameData FrameDatas[RPG_FRAME_BUFFERING];


	static DWORD Main(_In_ LPVOID lpParameter)
	{
		RPG_Log(RpgLogSystem, "[Thread-render] running...");

		static int FrameCounter = 0;

		while (bRunning)
		{
			const int frameIndex = FrameCounter % RPG_FRAME_BUFFERING;
			//RPG_Log(RpgLogSystem, "[Thread-render] Wait signal for frame (%i)", frameIndex);

			WaitForSingleObject(AwakeSemaphore, INFINITE);

			if (!bRunning)
			{
				break;
			}

			//RPG_Log(RpgLogSystem, "[Thread-render] Execute frame (%i)", frameIndex);

			FFrameData& frame = FrameDatas[frameIndex];
			RPG_Check(frame.Renderer);
			frame.Renderer->Execute(FrameCounter, frameIndex, frame.DeltaTime);
			frame.Renderer = nullptr;

			//RPG_Log(RpgLogSystem, "[Thread-render] Finish frame (%i)", frameIndex);
			++FrameCounter;

			ReleaseSemaphore(frame.DoneSemaphore, 1, NULL);

			if (!bRunning)
			{
				break;
			}
		}

		RPG_Log(RpgLogSystem, "[Thread-render] exit");

		return 0;
	}

};



void RpgRenderThread::Initialize() noexcept
{
	if (bRunning)
	{
		return;
	}

	bRunning = true;


#if RPG_RENDER_MULTITHREADED
	AwakeSemaphore = CreateSemaphoreA(NULL, 0, RPG_FRAME_BUFFERING, NULL);

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FrameDatas[f].DoneSemaphore = CreateSemaphoreA(NULL, 1, RPG_FRAME_BUFFERING, NULL);
	}
	
	Handle = CreateThread(NULL, 0, RpgRenderThread::Main, NULL, 0, NULL);

	SetThreadAffinityMask(Handle, (DWORD_PTR)(1 << 1));
#endif // RPG_RENDER_MULTITHREADED

}


void RpgRenderThread::Shutdown() noexcept
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;


#if RPG_RENDER_MULTITHREADED
	ReleaseSemaphore(AwakeSemaphore, 1, NULL);

	WaitForSingleObject(Handle, INFINITE);
	CloseHandle(Handle);
	Handle = NULL;

	CloseHandle(AwakeSemaphore);
	AwakeSemaphore = NULL;

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		CloseHandle(FrameDatas[f].DoneSemaphore);
		FrameDatas[f].DoneSemaphore = NULL;
	}
#endif // RPG_RENDER_MULTITHREADED

}


void RpgRenderThread::WaitFrame(int frameIndex) noexcept
{
#if RPG_RENDER_MULTITHREADED
	//RPG_Log(RpgLogSystem, "Wait render thread finish frame (%i)", frameIndex);
	FFrameData& frame = FrameDatas[frameIndex];
	WaitForSingleObject(frame.DoneSemaphore, INFINITE);
#endif // RPG_RENDER_MULTITHREADED

	//RPG_Log(RpgLogSystem, "Begin render frame (%i)", frameIndex);
}


void RpgRenderThread::ExecuteFrame(uint64_t frameCounter, int frameIndex, float deltaTime, RpgRenderer* renderer) noexcept
{
	FFrameData& frame = FrameDatas[frameIndex];
	frame.Renderer = renderer;
	frame.DeltaTime = deltaTime;

#if RPG_RENDER_MULTITHREADED
	//RPG_Log(RpgLogSystem, "Signal render thread execute frame (%i)", frameIndex);
	ReleaseSemaphore(AwakeSemaphore, 1, NULL);

#else
	frame.Renderer->Execute(frameCounter, frameIndex, deltaTime);

#endif // RPG_RENDER_MULTITHREADED

}
