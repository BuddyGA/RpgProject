#include "RpgGuiLayout.h"
#include "RpgGuiContext.h"
#include "render/RpgRenderer2D.h"



RpgGuiLayout::RpgGuiLayout(const RpgName& in_Name) noexcept
	: RpgGuiWidget(in_Name)
{
	Flags = RpgGui::FLAG_Layout;
	BackgroundColor = RpgColor::BLACK_TRANSPARENT;
}


RpgGuiLayout::RpgGuiLayout(const RpgName& in_Name, RpgPointFloat in_Dimension, EDirection in_Direction) noexcept
	: RpgGuiLayout(in_Name)
{
	Dimension = in_Dimension;
	Direction = in_Direction;
	ChildSpace = RpgPointFloat(4.0f);
	bScrollableHorizontal = false;
	bScrollableVertical = false;
}


RpgRectFloat RpgGuiLayout::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
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
				const RpgRect childRect = Children[c]->UpdateRect(context, canvasRect, childOffset);
				ContentRect.Right = RpgMath::Max(ContentRect.Right, childRect.Right);
				ContentRect.Bottom = RpgMath::Max(ContentRect.Bottom, childRect.Bottom);
			}
		}
		else if (Direction == DIRECTION_HORIZONTAL)
		{
			for (int c = 0; c < Children.GetCount(); ++c)
			{
				const RpgRect childRect = Children[c]->UpdateRect(context, canvasRect, RpgPointFloat(ContentRect.Right, ContentRect.Top));
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
				RpgGuiWidget* child = Children[c].Get();

				if (child->Dimension.X <= 0.0f)
				{
					child->Dimension.X = Dimension.X - ChildPadding.Right - ChildPadding.Left;
				}

				const RpgRect childRect = child->UpdateRect(context, canvasRect, RpgPointFloat(ContentRect.Left, ContentRect.Bottom));
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
		ApplyScroll(ScrollValue.X - context.MouseScrollValue.X * context.ScrollSpeed, ScrollValue.Y - context.MouseScrollValue.Y * context.ScrollSpeed);
	}

	if (PendingScrollValue.X > 0.0f || PendingScrollValue.Y > 0.0f)
	{
		ApplyScroll(PendingScrollValue.X, PendingScrollValue.Y);
		PendingScrollValue = RpgPointFloat();
	}

	return AbsoluteRect;
}


void RpgGuiLayout::SetScrollValue(float x, float y) noexcept
{
	PendingScrollValue = RpgPointFloat(x, y);
}


void RpgGuiLayout::OnRender(RpgRenderer2D& renderer) const noexcept
{
	RpgGuiWidget::OnRender(renderer);

	if (IsHovered())
	{
		renderer.AddLineRect(ContentRect, RpgColor::BLUE);
	}
}


void RpgGuiLayout::ApplyScroll(float x, float y) noexcept
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
		ScrollValue.X = 0.0f;
	}

	// Check if can scroll vertical
	if (bScrollableVertical && scrollDimension.Y > layoutDimension.Y)
	{
		ScrollValue.Y = RpgMath::Clamp(y, 0.0f, scrollDimension.Y + ChildPadding.Top + ChildPadding.Bottom - layoutDimension.Y);
	}
	else
	{
		ScrollValue.Y = 0.0f;
	}
}
