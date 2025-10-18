#pragma once

#include "HoverManager.h"

class DragManager : public HoverManager
{
private:
	Optional<int> m_heldIndex = none;
	Vec2 m_offset = Vec2{ 0, 0 };
	Vec2 m_currentPos = Vec2{ 0, 0 };
	bool m_isDragging = false;

public:
	DragManager() = default;

	void updateDrag(Array<RectF>& cardRects)
	{
		updateHover(cardRects);
		m_currentPos = Cursor::PosF();

		if (MouseL.down())
		{
			if (m_hoveredIndex)
			{
				m_heldIndex = m_hoveredIndex;
				m_offset = m_currentPos - cardRects[*m_heldIndex].center();
				m_isDragging = true;
			}
		}

		if (MouseL.up())
		{
			m_isDragging = false;
			m_heldIndex = none;
		}
	}

	bool isDragging() const { return m_isDragging; }
	Optional<int> heldIndex() const { return m_heldIndex; }
	Vec2 currentPos() const { return m_currentPos; }

	RectF getDraggingRect(double cardWidth, double cardHeight) const
	{
		if (!m_isDragging || !m_heldIndex) return RectF{ 0, 0, 0, 0 };
		return RectF{ Arg::center(m_currentPos - m_offset), cardWidth, cardHeight };
	}
};
