#pragma once


class RpgRenderer;


namespace RpgRenderThread
{
	extern void Initialize() noexcept;
	extern void Shutdown() noexcept;
	extern void WaitFrame(int frameIndex) noexcept;
	extern void ExecuteFrame(int frameIndex, float deltaTime, RpgRenderer* renderer) noexcept;

};
