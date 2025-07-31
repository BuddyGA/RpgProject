#include "RpgGuiLayout.h"
#include "render/RpgRenderer2D.h"



RpgGuiLayout::RpgGuiLayout() noexcept
{
	Flags = RpgGui::FLAG_Layout;

	ChildSpace = RpgPointFloat(4);
	bScrollableHorizontal = false;
	bScrollableVertical = false;
	Direction = DIRECTION_NONE;
}


RpgGuiLayout::RpgGuiLayout(const RpgName& in_Name, RpgPointFloat in_Dimension, EDirection in_Direction) noexcept
	: RpgGuiLayout()
{
	Name = in_Name;
	Dimension = in_Dimension;
	Direction = in_Direction;
}


RpgRectFloat RpgGuiLayout::UpdateRect(const RpgGuiContext& context, const RpgGuiCanvas& canvas, const RpgPointFloat& offset) noexcept
{
	AbsoluteRect = CalculateAbsoluteRect(offset);

	// Calculate content rect
	{
		ContentRect.Left = AbsoluteRect.Left + ChildPadding.Left - ScrollValue.X;
		ContentRect.Top = AbsoluteRect.Top + ChildPadding.Top - ScrollValue.Y;
		ContentRect.Right = ContentRect.Left;
		ContentRect.Bottom = ContentRect.Top;

		RpgPointFloat childOffset = ContentRect.GetPosition();

		if (Direction == DIRECTION_NONE)
		{
			for (int c = 0; c < Children.GetCount(); ++c)
			{
				const RpgRect childRect = Children[c]->UpdateRect(context, canvas, childOffset);
				ContentRect.Right = RpgMath::Max(ContentRect.Right, childRect.Right);
				ContentRect.Bottom = RpgMath::Max(ContentRect.Bottom, childRect.Bottom);
			}
		}
		else if (Direction == DIRECTION_HORIZONTAL)
		{
			for (int c = 0; c < Children.GetCount(); ++c)
			{
				const RpgRect childRect = Children[c]->UpdateRect(context, canvas, RpgPointFloat(ContentRect.Right, ContentRect.Top));
				ContentRect.Right += childRect.GetDimension().X + ChildSpace.X;
				ContentRect.Bottom = RpgMath::Max(ContentRect.Bottom, childRect.Bottom);
			}

			// Remove child space from last element
			if (!Children.IsEmpty())
			{
				ContentRect.Right -= ChildSpace.X;
			}
		}
		else // (Direction == DIRECTION_VERTICAL)
		{
			for (int c = 0; c < Children.GetCount(); ++c)
			{
				const RpgRect childRect = Children[c]->UpdateRect(context, canvas, RpgPointFloat(ContentRect.Left, ContentRect.Bottom));
				ContentRect.Right = RpgMath::Max(ContentRect.Right, childRect.Right);
				ContentRect.Bottom += childRect.GetDimension().Y + ChildSpace.Y;
			}

			// Remove child space from last element
			if (!Children.IsEmpty())
			{
				ContentRect.Bottom -= ChildSpace.Y;
			}
		}
	}

	
	if (IsHovered())
	{
		SetScrollValue(ScrollValue.X - context.MouseScrollValue.X * context.ScrollSpeed, ScrollValue.Y - context.MouseScrollValue.Y * context.ScrollSpeed);
	}

	return AbsoluteRect;
}


void RpgGuiLayout::SetScrollValue(float x, float y) noexcept
{
	const RpgPointFloat layoutDimension = AbsoluteRect.GetDimension();
	const RpgPointFloat scrollDimension = ContentRect.GetDimension();

	// Check if can scroll horizontal
	if (bScrollableHorizontal && scrollDimension.X > layoutDimension.X)
	{
		ScrollValue.X = RpgMath::Clamp(x, 0.0f, scrollDimension.X + ChildPadding.Left + ChildPadding.Right - layoutDimension.X);
	}
	else
	{
		ScrollValue.X = 0;
	}

	// Check if can scroll vertical
	if (bScrollableVertical && scrollDimension.Y > layoutDimension.Y)
	{
		ScrollValue.Y = RpgMath::Clamp(y, 0.0f, scrollDimension.Y + ChildPadding.Top + ChildPadding.Bottom - layoutDimension.Y);
	}
	else
	{
		ScrollValue.Y = 0;
	}
}


void RpgGuiLayout::OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept
{
	if (IsHovered())
	{
		renderer.AddLineRect(AbsoluteRect, RpgColorRGBA::WHITE);
		renderer.AddLineRect(ContentRect, RpgColorRGBA::BLUE);
	}
}
