#include "RpgAnimationWorldSubsystem.h"
#include "RpgAnimationComponent.h"
#include "render/RpgRenderer.h"
#include "render/RpgRenderer2D.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogAnimation, VERBOSITY_DEBUG)



RpgAnimationWorldSubsystem::RpgAnimationWorldSubsystem() noexcept
{
	Name = "AnimationWorldSubsystem";

	GlobalPlayRate = 1.0f;
	bDebugDrawSkeletonBones = false;
	bTickAnimationPose = false;
}


void RpgAnimationWorldSubsystem::StartPlay() noexcept
{
	bTickAnimationPose = true;
}


void RpgAnimationWorldSubsystem::StopPlay() noexcept
{
	bTickAnimationPose = false;
}


void RpgAnimationWorldSubsystem::TickUpdate(float deltaTime) noexcept
{
	if (!bTickAnimationPose)
	{
		return;
	}

	RpgWorld* world = GetWorld();

	RpgThreadTask* submitTasks[TASK_COUNT];

	// Reset tasks
	for (int i = 0; i < TASK_COUNT; ++i)
	{
		RpgAnimationTask_TickPose& task = TaskTickPoses[i];
		task.Reset();
		task.World = world;
		task.DeltaTime = deltaTime;
		task.GlobalPlayRate = GlobalPlayRate;

		submitTasks[i] = &task;
	}

	// Distribute tasks
	int taskIndex = 0;

	for (auto it = world->Component_CreateIterator<RpgAnimationComponent_AnimSkeletonPose>(); it; ++it)
	{
		RpgAnimationTask_TickPose& task = TaskTickPoses[taskIndex];
		task.AnimationComponents.AddValue(&it.GetValue());
		taskIndex = (taskIndex + 1) % TASK_COUNT;
	}

	RpgThreadPool::SubmitTasks(submitTasks, TASK_COUNT);
}


void RpgAnimationWorldSubsystem::Render(int frameIndex, RpgRenderer* renderer) noexcept
{
	RpgThreadTask* waitTasks[TASK_COUNT];
	for (int i = 0; i < TASK_COUNT; ++i)
	{
		waitTasks[i] = &TaskTickPoses[i];
	}

	// wait all task tick pose finished
	RPG_THREAD_TASK_WaitAll(waitTasks, TASK_COUNT);


#ifndef RPG_BUILD_SHIPPING
	RpgWorld* world = GetWorld();

	if (bDebugDrawSkeletonBones)
	{
		RpgVertexPrimitiveBatchLine* debugLine = renderer->Debug_GetPrimitiveBatchLine(frameIndex, world, true);

		for (auto it = world->Component_CreateIterator<RpgAnimationComponent_AnimSkeletonPose>(); it; ++it)
		{
			const RpgAnimationComponent_AnimSkeletonPose& comp = it.GetValue();
			if (!comp.Skeleton)
			{
				continue;
			}
			
			const RpgMatrixTransform gameObjectWorldMatrix = world->GameObject_GetWorldTransformMatrix(comp.GameObject);
			const RpgArray<int>& boneParentIndices = comp.Skeleton->GetBoneParentIndices();
			const RpgArray<RpgMatrixTransform>& bonePoseTransforms = comp.FinalPose.GetBonePoseTransforms();
			const int boneCount = boneParentIndices.GetCount();

			for (int b = 0; b < boneCount; ++b)
			{
				const RpgVector3 bonePosition = bonePoseTransforms[b].GetPosition() * gameObjectWorldMatrix;
				debugLine->AddAABB(RpgBoundingAABB(bonePosition - 2.0f, bonePosition + 2.0f), RpgColor::RED);

				const int boneParentIndex = boneParentIndices[b];
				
				if (boneParentIndex != RPG_SKELETON_BONE_INDEX_INVALID)
				{
					const RpgVector3 boneParentPosition = bonePoseTransforms[boneParentIndex].GetPosition() * gameObjectWorldMatrix;
					debugLine->AddLine(boneParentPosition, bonePosition, RpgColor::WHITE);
				}

				// Bone name
				
			}
		}
	}
#endif // !RPG_BUILD_SHIPPING
}
