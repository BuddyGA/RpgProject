#pragma once

#include "RpgGuiTypes.h"



// Base gui widget class (canvas, layout, control)
class RpgGuiWidget
{
	RPG_NOCOPY(RpgGuiWidget)

public:
	// Widget name
	RpgName Name;

	// Position offset from parent
	RpgPointFloat Position;

	// Dimension X (width) and Y (height)
	RpgPointFloat Dimension;

	uint8_t Order;


public:
	RpgGuiWidget() noexcept;
	RpgGuiWidget(const RpgName& in_Name) noexcept;
	virtual ~RpgGuiWidget() noexcept = default;

	virtual void Initialize() noexcept {}
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept;
	void UpdateState(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept;
	void Render(const RpgGuiContext& context, RpgRenderer2D& renderer, uint8_t parentOrder, const RpgRectFloat& parentClipRect) const noexcept;


	inline const RpgRectFloat& GetAbsoluteRect() const noexcept
	{
		return AbsoluteRect;
	}


	template<typename TWidget, typename...TConstructorArgs>
	inline TWidget* AddChild(TConstructorArgs&&... args) noexcept
	{
		static_assert(std::is_base_of<RpgGuiWidget, TWidget>::value, "RpgGuiWidget: AddChild type of <TWidget> must be derived from type <RpgGuiWidget>!");

		RPG_Check(IsLayout());

		const int index = Children.GetCount();
		Children.AddValue(RpgPointer::MakeUnique<TWidget>(std::forward<TConstructorArgs>(args)...));

		TWidget* child = static_cast<TWidget*>(Children[index].Get());

		if (child->Order == 255)
		{
			child->Order = Order - 1;
		}

		child->Initialize();

		return child;
	}

	inline void SetVisibility(bool bVisible) noexcept
	{
		RpgType::BitSetCondition<uint16_t>(Flags, RpgGui::FLAG_State_Invisible, !bVisible);
	}

	inline bool IsVisible() const noexcept
	{
		return !(Flags & RpgGui::FLAG_State_Invisible);
	}

	inline bool IsLayout() const noexcept
	{
		return Flags & RpgGui::FLAG_Layout;
	}

	inline bool HasInputFocus() const noexcept
	{
		return Flags & RpgGui::FLAG_InputFocus;
	}

	inline bool IsHovered() const noexcept
	{
		return Flags & RpgGui::FLAG_State_Hovered;
	}

	inline bool IsPressed() const noexcept
	{
		return Flags & RpgGui::FLAG_State_Pressed;
	}

	inline bool IsReleased() const noexcept
	{
		return Flags & RpgGui::FLAG_State_Released;
	}

	inline bool IsFocused() const noexcept
	{
		return Flags & RpgGui::FLAG_State_Focused;
	}


	inline RpgRectFloat CalculateAbsoluteRect(const RpgPointFloat& offset) const noexcept
	{
		RpgRectFloat rect;
		rect.Left = Position.X + offset.X;
		rect.Top = Position.Y + offset.Y;
		rect.Right = rect.Left + Dimension.X;
		rect.Bottom = rect.Top + Dimension.Y;

		return rect;
	}


protected:
	virtual void OnHoveredEnter() noexcept {}
	virtual void OnHoveredExit() noexcept {}
	virtual void OnPressed() noexcept {}
	virtual void OnReleased() noexcept {}
	virtual void OnFocusedEnter() noexcept {}
	virtual void OnFocusedExit() noexcept {}
	virtual void OnUpdate(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept {}
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept;


protected:
	RpgRectFloat AbsoluteRect;
	RpgArray<RpgUniquePtr<RpgGuiWidget>> Children;
	uint16_t Flags;


private:
	inline void HoveredEnter() noexcept
	{
		if (!(Flags & RpgGui::FLAG_State_Hovered))
		{
			RPG_LogDebug(RpgLogGui, "%s hovered (enter) (order: %u)", *Name, Order);
			Flags |= RpgGui::FLAG_State_Hovered;
			OnHoveredEnter();
		}
	}

	inline void HoveredExit() noexcept
	{
		if (Flags & RpgGui::FLAG_State_Hovered)
		{
			RPG_LogDebug(RpgLogGui, "%s hovered (exit)", *Name);
			Flags &= ~RpgGui::FLAG_State_Hovered;
			OnHoveredExit();
		}
	}

	inline void Pressed() noexcept
	{
		if (!(Flags & RpgGui::FLAG_State_Pressed))
		{
			RPG_LogDebug(RpgLogGui, "%s pressed", *Name);
			Flags |= RpgGui::FLAG_State_Pressed;
			OnPressed();
		}
	}

	inline void Released() noexcept
	{
		if (!(Flags & RpgGui::FLAG_State_Released))
		{
			const bool bWasPressed = (Flags & RpgGui::FLAG_State_Pressed);
			RPG_Check(bWasPressed);
			Flags &= ~RpgGui::FLAG_State_Pressed;

			RPG_LogDebug(RpgLogGui, "%s released", *Name);
			Flags |= RpgGui::FLAG_State_Released;
			OnReleased();
		}
	}

	inline void FocusedEnter() noexcept
	{
		if (!(Flags & RpgGui::FLAG_State_Focused))
		{
			RPG_LogDebug(RpgLogGui, "%s focused (enter)", *Name);
			Flags |= RpgGui::FLAG_State_Focused;
			OnFocusedEnter();
		}
	}

	inline void FocusedExit() noexcept
	{
		if (Flags & RpgGui::FLAG_State_Focused)
		{
			RPG_LogDebug(RpgLogGui, "%s focused (exit)", *Name);
			Flags &= ~RpgGui::FLAG_State_Focused;
			OnFocusedExit();
		}
	}

	friend RpgGuiContext;

};
