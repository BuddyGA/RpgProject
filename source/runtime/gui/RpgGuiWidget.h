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
	void UpdateState(RpgGuiContext& context) noexcept;
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
			child->Order = child->IsLayout() ? Order - 1 : Order;
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
	virtual void OnHoveredEnter(RpgGuiContext& context) noexcept {}
	virtual void OnHoveredExit(RpgGuiContext& context) noexcept {}
	virtual void OnPressed(RpgGuiContext& context) noexcept {}
	virtual void OnReleased(RpgGuiContext& context) noexcept {}
	virtual void OnFocusedEnter(RpgGuiContext& context) noexcept {}
	virtual void OnFocusedExit(RpgGuiContext& context) noexcept {}
	virtual void OnUpdate(RpgGuiContext& context) noexcept {}
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept {};


protected:
	RpgRectFloat AbsoluteRect;
	RpgArray<RpgUniquePtr<RpgGuiWidget>> Children;
	uint16_t Flags;


	friend RpgGuiContext;

};
