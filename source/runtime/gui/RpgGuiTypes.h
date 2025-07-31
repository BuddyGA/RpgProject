#pragma once

#include "core/RpgDelegate.h"
#include "input/RpgInputTypes.h"
#include "render/RpgFont.h"
#include "render/RpgMaterial.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogGui)


#define RPG_GUI_SCROLL_VALUE_FIRST	0
#define RPG_GUI_SCROLL_VALUE_LAST	-1


class RpgGuiWidget;
class RpgGuiCanvas;
class RpgGuiContext;



enum class RpgGuiAlignHorizontal : uint8_t
{
	LEFT = 0,
	CENTER,
	RIGHT
};



enum class RpgGuiAlignVertical : uint8_t
{
	TOP = 0,
	CENTER,
	BOTTOM
};



namespace RpgGui
{
	enum Flags : uint16_t
	{
		FLAG_None				= (0),
		FLAG_Layout				= (1 << 0),
		FLAG_InputFocus			= (1 << 1),
		FLAG_State_Invisible	= (1 << 2),
		FLAG_State_Hovered		= (1 << 3),
		FLAG_State_Pressed		= (1 << 4),
		FLAG_State_Released		= (1 << 5),
		FLAG_State_Focused		= (1 << 6),
	};


	extern RpgPointFloat CalculateTextAlignmentPosition(const RpgRectFloat& rect, const RpgString& text, const RpgSharedFont& font, RpgGuiAlignHorizontal alignH, RpgGuiAlignVertical alignV) noexcept;
}



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
	virtual ~RpgGuiWidget() noexcept = default;

	virtual void Initialize() noexcept {}
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgGuiCanvas& canvas, const RpgPointFloat& offset) noexcept;
	void UpdateState(RpgGuiContext& context) noexcept;
	void Render(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept;


	inline const RpgRectFloat& GetAbsoluteRect() const noexcept
	{
		return AbsoluteRect;
	}


	template<typename TWidget, typename...TConstructorArgs>
	inline TWidget* AddChild(TConstructorArgs&&... args) noexcept
	{
		static_assert(std::is_base_of<RpgGuiWidget, TWidget>::value, "RpgGuiWidget: AddChild type of <TWidget> must be derived from type <RpgGuiWidget>!");

		const int index = Children.GetCount();
		Children.AddValue(RpgPointer::MakeUnique<TWidget>(std::forward<TConstructorArgs>(args)...));
		
		TWidget* child = static_cast<TWidget*>(Children[index].Get());
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
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept {};


protected:
	RpgRectFloat AbsoluteRect;
	RpgArray<RpgUniquePtr<RpgGuiWidget>> Children;
	uint16_t Flags;


	friend RpgGuiContext;

};




class RpgGuiCanvas
{
public:
	RpgName Name;


public:
	RpgGuiCanvas() noexcept = default;
	void Update(RpgGuiContext& context, RpgRectFloat rect) noexcept;


	template<typename TWidget, typename...TConstructorArgs>
	inline TWidget* AddChild(TConstructorArgs&&... args) noexcept
	{
		return RootWidget.AddChild<TWidget>(std::forward<TConstructorArgs>(args)...);
	}


	inline void Render(const RpgGuiContext& context, RpgRenderer2D& renderer) noexcept
	{
		RootWidget.Render(context, renderer, RootWidget.GetAbsoluteRect());
	}

	inline RpgRectFloat GetRect() const noexcept
	{
		return RootWidget.GetAbsoluteRect();
	}


private:
	RpgGuiWidget RootWidget;

};




class RpgGuiContext
{
public:
	RpgGuiContext() noexcept;


public:
	RpgPointFloat MouseCursorPosition;
	RpgPointFloat MouseCursorDeltaPosition;
	RpgPointFloat MouseScrollValue;
	float ScrollSpeed;
	char TextInputChar;

	RpgGuiWidget* PrevHovered;
	RpgGuiWidget* PrevHoveredLayout;
	RpgGuiWidget* PrevPressed;
	RpgGuiWidget* CurrentHovered;
	RpgGuiWidget* CurrentHoveredLayout;
	RpgGuiWidget* CurrentPressed;
	RpgGuiWidget* CurrentFocused;


public:
	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;
	void CharInput(char c) noexcept;

	void Begin() noexcept;
	void End() noexcept;
	void BeginTextInput();
	void EndTextInput();


	inline bool IsMouseButtonAnyDown() const noexcept
	{
		return IsKeyButtonDown(RpgInputKey::MOUSE_LEFT) || IsKeyButtonDown(RpgInputKey::MOUSE_MIDDLE) || IsKeyButtonDown(RpgInputKey::MOUSE_RIGHT);
	}

	inline bool IsKeyButtonDown(RpgInputKey::EButton button) const noexcept
	{
		return KeyButtonDown[button];
	}

	inline bool IsKeyButtonUp(RpgInputKey::EButton button) const noexcept
	{
		return KeyButtonUp[button];
	}

	inline void SetHoveredLayout(RpgGuiWidget* layout) noexcept
	{
		RPG_Check(layout);

		if (PrevHoveredLayout == nullptr || PrevHoveredLayout->Order <= layout->Order)
		{
			PrevHoveredLayout = layout;
		}
	}

	inline void SetFocusWidget(RpgGuiWidget* widget) noexcept
	{
		if (CurrentFocused != widget && widget->HasInputFocus())
		{
			widget->Flags |= RpgGui::FLAG_State_Focused;
			widget->OnFocusedEnter(*this);
			CurrentFocused = widget;
		}
	}


private:
	bool KeyButtonDown[RpgInputKey::MAX_COUNT];
	bool KeyButtonUp[RpgInputKey::MAX_COUNT];

};
