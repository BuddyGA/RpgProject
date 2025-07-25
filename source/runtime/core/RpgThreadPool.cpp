#include "RpgThreadPool.h"
#include "dsa/RpgArray.h"



namespace RpgThreadPool
{
	class FTaskQueue
	{
	private:
		RpgArray<RpgThreadTask*, 8> Pool;
		CRITICAL_SECTION CS;


	public:
		FTaskQueue() noexcept
		{
			InitializeCriticalSection(&CS);
		}

		~FTaskQueue() noexcept
		{
			DeleteCriticalSection(&CS);
		}

		inline void PushTasks(RpgThreadTask** tasks, int count) noexcept
		{
			EnterCriticalSection(&CS);
			Pool.InsertAtRange(tasks, count, RPG_INDEX_LAST);
			LeaveCriticalSection(&CS);
		}

		inline RpgThreadTask* PopTask() noexcept
		{
			EnterCriticalSection(&CS);
			RpgThreadTask* task = nullptr;

			if (Pool.GetCount() > 0)
			{
				task = Pool[0];
				Pool.RemoveAt(0);
			}

			LeaveCriticalSection(&CS);

			return task;
		}

	};


	static HANDLE SignalSemaphore;
	static FTaskQueue TaskQueue;


	struct FThreadWorker
	{
		char Name[32];
		HANDLE Handle{ nullptr };
		RpgAtomicInt IsRunning{};
	};
	static RpgArrayInline<FThreadWorker, 32> ThreadWorkers;

	static bool bInitialized;


	static DWORD ThreadWorker_Main(_In_ LPVOID lpParameter)
	{
		RpgThreadPool::FThreadWorker* worker = reinterpret_cast<RpgThreadPool::FThreadWorker*>(lpParameter);

		while (worker->IsRunning)
		{
			WaitForSingleObject(SignalSemaphore, INFINITE);

			// Check if should exit
			if (!worker->IsRunning)
			{
				break;
			}

			RpgThreadTask* task = TaskQueue.PopTask();

			while (task)
			{
				//RPG_PLATFORM_LogDebug(RpgLogSystem, "%s execute task %s", threadName, task->GetTaskName());
				task->Execute();
				task->SetDone();

				// Check if should exit
				if (!worker->IsRunning)
				{
					break;
				}

				task = TaskQueue.PopTask();
			}

			//RPG_PLATFORM_LogDebug(RpgLogSystem, "%s back to wait", threadName);
		}

		RPG_Log(RpgLogSystem, "%s exit", worker->Name);

		return 0;
	}

};


void RpgThreadPool::Initialize(int numOtherDedicatedThreads) noexcept
{
	if (bInitialized)
	{
		return;
	}
	
	SYSTEM_INFO systemInfo{};
	GetSystemInfo(&systemInfo);

	const int cpuCount = systemInfo.dwNumberOfProcessors;
	RPG_RuntimeErrorCheck(cpuCount >= 4, "CPU must have at least 4 cores!");

	// Exclude dedicated threads and main thread
	const int numThreadWorkers = cpuCount - numOtherDedicatedThreads - 1;
	RPG_Validate(numThreadWorkers > 1);

	RPG_Log(RpgLogSystem, "Initialize threadpool with %i worker threads", numThreadWorkers);

	SignalSemaphore = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
	ThreadWorkers.Resize(numThreadWorkers);

	for (int i = 0; i < numThreadWorkers; ++i)
	{
		FThreadWorker& worker = ThreadWorkers[i];
		snprintf(worker.Name, 32, "Thread-Worker-%i", i);
		worker.IsRunning = 1;
		worker.Handle = CreateThread(NULL, 0, ThreadWorker_Main, &worker, CREATE_SUSPENDED, NULL);
	}

	for (int i = 0; i < numThreadWorkers; ++i)
	{
		ResumeThread(ThreadWorkers[i].Handle);
	}
	
	bInitialized = true;
}


void RpgThreadPool::Shutdown() noexcept
{
	if (!bInitialized)
	{
		return;
	}

	RPG_Log(RpgLogSystem, "Shutdown threadpool");

	for (int t = 0; t < ThreadWorkers.GetCount(); ++t)
	{
		FThreadWorker& worker = ThreadWorkers[t];
		InterlockedExchange(&worker.IsRunning, 0);
	}

	ReleaseSemaphore(SignalSemaphore, ThreadWorkers.GetCount(), NULL);

	for (int t = 0; t < ThreadWorkers.GetCount(); ++t)
	{
		FThreadWorker& worker = ThreadWorkers[t];
		WaitForSingleObject(worker.Handle, INFINITE);
		CloseHandle(worker.Handle);
	}

	ThreadWorkers.Clear();

	CloseHandle(SignalSemaphore);
	SignalSemaphore = nullptr;

	bInitialized = false;
}


void RpgThreadPool::SubmitTasks(RpgThreadTask** tasks, int taskCount) noexcept
{
	RPG_Assert(tasks && taskCount > 0);

	for (int i = 0; i < taskCount; ++i)
	{
		RpgThreadTask* task = tasks[i];
		RPG_CheckV(task->IsIdle(), "Task submitted must be on idle state!");
		task->SetRunning();
	}

	TaskQueue.PushTasks(tasks, taskCount);

	ReleaseSemaphore(SignalSemaphore, taskCount, NULL);
}
