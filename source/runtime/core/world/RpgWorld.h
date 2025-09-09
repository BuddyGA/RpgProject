#pragma once

#include "RpgLevel.h"



RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogWorld)


class RpgWorld;
class RpgWorldSubsystem;
class RpgRenderer;



class RpgWorldSubsystem
{
	RPG_NOCOPY(RpgWorldSubsystem)

public:
	RpgWorldSubsystem() noexcept
	{
		World = nullptr;
	}

	virtual ~RpgWorldSubsystem() noexcept = default;

protected:
	virtual void StartPlay() noexcept {}
	virtual void StopPlay() noexcept {}
	virtual void PreTickUpdate() noexcept {}
	virtual void TickUpdate(float deltaTime) noexcept {}
	virtual void PostTickUpdate() noexcept {}
	virtual void Render(int frameIndex, RpgRenderer* renderer) noexcept {}


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline RpgWorld* GetWorld() const noexcept
	{
		return World;
	}


protected:
	RpgName Name;

private:
	RpgWorld* World;


	friend RpgWorld;

};




class RpgWorld
{
	RPG_NOCOPY(RpgWorld)

public:
	RpgWorld(const RpgName& in_Name) noexcept;
	virtual void Initialize() noexcept;

	void BeginFrame(int frameIndex) noexcept;
	void EndFrame(int frameIndex) noexcept;
	void DispatchStartPlay() noexcept;
	void DispatchStopPlay() noexcept;
	void DispatchTickUpdate(float deltaTimeSeconds) noexcept;
	void DispatchPostTickUpdate() noexcept;
	void DispatchRender(int frameIndex, RpgRenderer* renderer) noexcept;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline bool HasStartedPlay() const noexcept
	{
		return bHasStartedPlay;
	}


private:
	RpgName Name;
	bool bHasStartedPlay;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Subsystem interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	template<typename TWorldSubsystem>
	inline void Subsystem_Register() noexcept
	{
		static_assert(std::is_base_of<RpgWorldSubsystem, TWorldSubsystem>::value, "RpgWorld: Add subsystem type of <TWorldSubsystem> must be derived from type <RpgWorldSubsystem>!");

		for (int i = 0; i < Subsystems.GetCount(); ++i)
		{
			if (TWorldSubsystem* check = dynamic_cast<TWorldSubsystem*>(Subsystems[i].Get()))
			{
				RPG_LogWarn(RpgLogWorld, "World subsystem type (%s) already exists!", *check->GetName());
				return;
			}
		}

		const int index = Subsystems.GetCount();
		Subsystems.AddValue(RpgPointer::MakeUnique<TWorldSubsystem>());

		RpgWorldSubsystem* subsystem = Subsystems[index].Get();
		subsystem->World = this;
	}


	template<typename TWorldSubsystem>
	inline TWorldSubsystem* Subsystem_Get() const noexcept
	{
		static_assert(std::is_base_of<RpgWorldSubsystem, TWorldSubsystem>::value, "RpgWorld: Get subsystem type of <TWorldSubsystem> must be derived from type <RpgWorldSubsystem>!");

		for (int i = 0; i < Subsystems.GetCount(); ++i)
		{
			if (const TWorldSubsystem* check = dynamic_cast<const TWorldSubsystem*>(Subsystems[i].Get()))
			{
				return const_cast<TWorldSubsystem*>(check);
			}
		}

		return nullptr;
	}


private:
	RpgArrayInline<RpgUniquePtr<RpgWorldSubsystem>, 16> Subsystems;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Level interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	void SaveLevel(const RpgName& name) noexcept;
	void LoadLevelAsync(const RpgString& path) noexcept;
	

	// Create gameobject. This is only allocate the gameobject in memory, to actually spawn it call RpgGameObject::SpawnAtTransform after finished (e.g. add component/script/attach to parent)
	// @param name - Name of the gameobject
	// @param opt_Level - (Optional) level owning the gameobject, if NULL main level will own the gameobject
	// @param opt_bIsTransient - (Optional) Set TRUE to mark gameobject as transient. Transient gameobject ignores serialization while saving level
	// @return Gameobject handle
	[[nodiscard]] RpgGameObject CreateGameObject(const RpgName& name, RpgLevel* opt_Level = nullptr, bool opt_bIsTransient = false) noexcept;


protected:
	virtual void RegisterComponents(RpgLevel* level) noexcept {}


private:
	RpgArray<uint64_t> LevelStreamingHashes;
	RpgArray<RpgString> LevelStreamingPaths;
	RpgArray<RpgUniquePtr<RpgLevel>> LevelLoadeds;


private:
	template<typename TComponent>
	class FComponentIterator
	{
		using Iterator = RpgFreeList<TComponent>::Iterator;

	private:
		FComponentIterator(RpgArray<Iterator>& in_Iterators) noexcept
			: LevelIterators(std::move(in_Iterators))
			, LevelIndex(RPG_INDEX_INVALID)
		{
			if (!LevelIterators.IsEmpty())
			{
				LevelIndex = 0;
				Current = LevelIterators[0];
			}
		}

		FComponentIterator(RpgArray<Iterator>&& in_Iterators) noexcept
			: LevelIterators(std::move(in_Iterators))
			, LevelIndex(RPG_INDEX_INVALID)
		{
			if (!LevelIterators.IsEmpty())
			{
				LevelIndex = 0;
				Current = LevelIterators[0];
			}
		}

	public:
		inline TComponent& GetValue() noexcept
		{
			return Current.GetValue();
		}


	public:
		inline Iterator& operator++() noexcept
		{
			++Current;

			if (!Current)
			{
				++LevelIndex;

				if (LevelIndex < LevelIterators.GetCount())
				{
					Current = LevelIterators[LevelIndex];
				}
			}

			return Current;
		}

		inline TComponent& operator*() noexcept
		{
			return *Current;
		}

		inline bool operator==(const Iterator& rhs) const noexcept
		{
			return Current == rhs;
		}

		inline bool operator!=(const Iterator& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		inline operator bool() const noexcept
		{
			return !LevelIterators.IsEmpty() && (LevelIndex >= 0 && LevelIndex < LevelIterators.GetCount()) && Current.IsValid();
		}


	private:
		RpgArray<Iterator> LevelIterators;
		int LevelIndex;
		Iterator Current;


		friend RpgWorld;

	};


	template<typename TComponent>
	class FComponentConstIterator
	{
		using Iterator = RpgFreeList<TComponent>::ConstIterator;

	private:
		FComponentConstIterator(RpgArray<Iterator>& in_Iterators) noexcept
			: LevelIterators(std::move(in_Iterators))
			, LevelIndex(RPG_INDEX_INVALID)
		{
			if (!LevelIterators.IsEmpty())
			{
				LevelIndex = 0;
				Current = LevelIterators[0];
			}
		}

		FComponentConstIterator(RpgArray<Iterator>&& in_Iterators) noexcept
			: LevelIterators(std::move(in_Iterators))
			, LevelIndex(RPG_INDEX_INVALID)
		{
			if (!LevelIterators.IsEmpty())
			{
				LevelIndex = 0;
				Current = LevelIterators[0];
			}
		}

	public:
		inline const TComponent& GetValue() noexcept
		{
			return Current.GetValue();
		}


	public:
		inline Iterator& operator++() noexcept
		{
			++Current;

			if (!Current)
			{
				++LevelIndex;

				if (LevelIndex < LevelIterators.GetCount())
				{
					Current = LevelIterators[LevelIndex];
				}
			}

			return Current;
		}

		inline const TComponent& operator*() noexcept
		{
			return *Current;
		}

		inline bool operator==(const Iterator& rhs) const noexcept
		{
			return Current == rhs;
		}

		inline bool operator!=(const Iterator& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		inline operator bool() const noexcept
		{
			return !LevelIterators.IsEmpty() && (LevelIndex >= 0 && LevelIndex < LevelIterators.GetCount()) && Current.IsValid();
		}


	private:
		RpgArray<Iterator> LevelIterators;
		int LevelIndex;
		Iterator Current;


		friend RpgWorld;

	};


public:
	template<typename TComponent>
	inline FComponentIterator<TComponent> ComponentIterator() noexcept
	{
		RpgArray<typename RpgFreeList<TComponent>::Iterator> levelCompIterators;
		levelCompIterators.Reserve(LevelLoadeds.GetCount());

		for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
		{
			levelCompIterators.AddValue(LevelLoadeds[i]->Component_Iterator<TComponent>());
		}

		return FComponentIterator<TComponent>(levelCompIterators);
	}


	template<typename TComponent>
	inline FComponentConstIterator<TComponent> ComponentConstIterator() const noexcept
	{
		RpgArray<typename RpgFreeList<TComponent>::ConstIterator> levelCompIterators;
		levelCompIterators.Reserve(LevelLoadeds.GetCount());

		for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
		{
			levelCompIterators.AddValue(LevelLoadeds[i]->Component_ConstIterator<TComponent>());
		}

		return FComponentConstIterator<TComponent>(levelCompIterators);
	}

};
