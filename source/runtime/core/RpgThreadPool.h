#pragma once

#include "RpgPlatform.h"



class RpgThreadTask
{
	RPG_NOCOPYMOVE(RpgThreadTask)

public:
	RpgThreadTask() noexcept
		: State()
	{
	}

	virtual ~RpgThreadTask() noexcept
	{
		RPG_Assert(IsIdle() || IsDone());
	}

	virtual void Reset() noexcept
	{
		RPG_AssertV(State != 1, "Cannot reset while it's still running!");
		InterlockedExchange(&State, 0);
	}

	virtual void Execute() noexcept = 0;
	virtual const char* GetTaskName() const noexcept { return nullptr; }


	// Called by threadpool when submit task. Do not call this manually!
	inline void SetRunning() noexcept
	{
		InterlockedExchange(&State, 1);
	}

	// Called by threadpool worker thread when task finish executed. Do not call this manually
	inline void SetDone() noexcept
	{
		InterlockedExchange(&State, 2);
	}


	// [Block] Wait until task finished.
	// @returns None
	inline void Wait() noexcept
	{
		while (State == 1);
		RPG_Assert(State == 2);
	}


	// Check if task is in idle state
	inline bool IsIdle() const noexcept
	{
		return State == 0;
	}

	// Check if task is in running state (submitted or probably being executed by worker thread)
	inline bool IsRunning() const noexcept
	{
		return State == 1;
	}

	// Check if task is done (worker thread has finished execute this task)
	inline bool IsDone() const noexcept
	{
		return State == 2;
	}


private:
	// [0]: Idle, [1]: Running, [2]: Done
	RpgAtomicInt State;

};


#define RPG_THREAD_TASK_WaitAll(taskArray, taskCount)	\
for (int __i = 0; __i < taskCount; ++__i)				\
{														\
	if (taskArray[__i]->IsRunning())					\
	{													\
		taskArray[__i]->Wait();							\
	}													\
}




namespace RpgThreadPool
{
	// Initialize thread pool with number worker threads at max (NumCore - numOtherDedicatedThreads - 1) depends on if there's dedicated render/audio thread
	// @param numOtherDedicatedThreads - Number of other dedicated threads
	// @returns None
	void Initialize(int numOtherDedicatedThreads = 0) noexcept;


	// Shutdown threadpool
	// @returns None
	void Shutdown() noexcept;


	// Submit task. 
	// @param task - Task to submit
	// @returns None
	void SubmitTasks(RpgThreadTask** tasks, int taskCount) noexcept;


	// [Block] Wait all tasks
	// @param tasks - Pointer to task data array
	// @param taskCount - Number of task count
	// @returns None
	//void WaitAllTasks(RpgThreadTask** tasks, int taskCount) noexcept;


	// Submit <tasks> into threadpool or execute in serial based on <bCondition>
	template<typename bool bCondition = false>
	inline void SubmitOrExecuteTasks(RpgThreadTask** tasks, int taskCount) noexcept
	{
		if (tasks == nullptr || taskCount == 0)
		{
			return;
		}

		if constexpr (bCondition)
		{
			SubmitTasks(tasks, taskCount);
		}
		else
		{
			for (int i = 0; i < taskCount; ++i)
			{
				RpgThreadTask* task = tasks[i];
				task->SetRunning();
				task->Execute();
				task->SetDone();
			}
		}
	}

};
