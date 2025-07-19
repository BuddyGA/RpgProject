#pragma once

#include "RpgPlatform.h"



class RpgTimer
{
public:
	// Micro seconds
	static constexpr int TIME_UNIT = 1000000;


public:
	RpgTimer() noexcept
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);

		PerformanceFrequency = static_cast<float>(li.QuadPart);
		StartTickCounter = 0;
		PrevTickCounter = 0;
		DeltaTime = 0.0f;
	}


	// Reset and stop the timer
	inline void Reset() noexcept
	{
		StartTickCounter = 0;
		PrevTickCounter = 0;
		DeltaTime = 0.0f;
	}


	// Start the timer if it hasn't started
	inline void Start() noexcept
	{
		if (StartTickCounter == 0)
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);

			StartTickCounter = li.QuadPart;
			PrevTickCounter = StartTickCounter;
		}
	}


	// Tick the timer
	// @returns Delta time in micro seconds
	inline float Tick() noexcept
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);

		const uint64_t currentTickCounter = li.QuadPart;
		DeltaTime = static_cast<float>((currentTickCounter - PrevTickCounter) * TIME_UNIT) / PerformanceFrequency;
		PrevTickCounter = currentTickCounter;

		return DeltaTime;
	}


	inline uint64_t GetCurrentTickCounter() const noexcept
	{
		return PrevTickCounter;
	}


	// Get delta time in seconds
	// @returns Delta time in seconds
	inline float GetDeltaTimeSeconds() const noexcept
	{
		return DeltaTime / TIME_UNIT;
	}


private:
	float PerformanceFrequency;
	uint64_t StartTickCounter;
	uint64_t PrevTickCounter;
	float DeltaTime;

};
