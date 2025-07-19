#pragma once

#include "RpgString.h"


class RpgState;
class RpgStateMachine;



class RpgState
{
	RPG_NOCOPYMOVE(RpgState)

public:
	RpgState(const RpgName& in_Name) noexcept
	{
		Name = in_Name;
		bTickUpdate = false;
	}

	virtual ~RpgState() noexcept = default;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}


protected:
	virtual bool CanEnterState(RpgStateMachine& stateMachine) const noexcept { return true; }
	virtual void OnEnterState(RpgStateMachine& stateMachine) noexcept {}
	virtual void OnLeaveState(RpgStateMachine& stateMachine) noexcept {}
	virtual void OnTickUpdateState(RpgStateMachine& stateMachine, float deltaTime) {}


protected:
	RpgName Name;
	bool bTickUpdate;


	friend class RpgStateMachine;

};



class RpgStateMachine
{
	RPG_NOCOPYMOVE(RpgStateMachine)

public:
	RpgStateMachine() noexcept
	{
		CurrentState = nullptr;
	}


	virtual ~RpgStateMachine() noexcept
	{
		for (int i = 0; i < States.GetCount(); ++i)
		{
			delete States[i];
		}
	}


	template<typename TState, typename...TConstructorArgs>
	inline TState* AddState(TConstructorArgs&&... args) noexcept
	{
		static_assert(std::is_base_of<RpgState, TState>::value, "RpgStateMachine::AddState type of <TState> must be derived from type <RpgState>!");

		TState* state = new TState(std::forward<TConstructorArgs>(args)...);
		States.AddUnique(state);

		return state;
	}


	inline void SetState(const RpgName& name) noexcept
	{
		RpgState* newState = nullptr;

		for (int i = 0; i < States.GetCount(); ++i)
		{
			if (States[i]->Name == name)
			{
				newState = States[i];
				break;
			}
		}

		RPG_Assert(newState && newState != CurrentState);

		if (CurrentState)
		{
			CurrentState->OnLeaveState(*this);
		}

		CurrentState = newState;
		RPG_Assert(CurrentState);
		CurrentState->OnEnterState(*this);
	}

	template<typename TState = RpgState>
	inline TState* GetCurrentState() noexcept
	{
		return dynamic_cast<TState*>(CurrentState);
	}


	inline void TickUpdate(float deltaTime) noexcept
	{
		if (States.IsEmpty())
		{
			return;
		}

		RPG_Assert(CurrentState);

		if (CurrentState->bTickUpdate)
		{
			CurrentState->OnTickUpdateState(*this, deltaTime);
		}
	}


private:
	RpgArrayInline<RpgState*, 32> States;
	RpgState* CurrentState;

};
