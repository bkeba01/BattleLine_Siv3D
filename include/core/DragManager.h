#pragma once

#include "HoverManager.h"

class DragManager : public HoverManager
{
private:
	Optional<int> m_heldIndex = none;
	Optional<int> m_heldIndexLastFrame = none;
	Vec2 m_offset = Vec2{ 0, 0 };
	Vec2 m_currentPos = Vec2{ 0, 0 };
	bool m_isDragging = false;
	bool m_DraggingLastFrame = false;
public:
	DragManager() = default;

	void updateDrag(Array<RectF>& cardRects)
	{
		
		m_DraggingLastFrame = m_isDragging;
		m_heldIndexLastFrame = m_heldIndex;
		printf("LastFrame: isDragging=%d, heldIndex=%s\n", m_DraggingLastFrame, m_heldIndexLastFrame ? std::to_string(*m_heldIndexLastFrame).c_str() : "none");
		updateHover(cardRects);
		m_currentPos = Cursor::PosF();

		if (MouseL.down())
		{
			printf("Dragging\n");
			if (m_hoveredIndex)
			{
				m_heldIndex = m_hoveredIndex;
				m_offset = m_currentPos - cardRects[*m_heldIndex].center();
				m_isDragging = true;
			}
		}
		else if (MouseL.up())
		{
			printf("Stop Dragging\n");
			m_isDragging = false;
			m_heldIndex = none;
		}
	}
	bool getDraggingLastFrame() const { return m_DraggingLastFrame; }
	Optional<int> getheldIndexLastFrame() const { return m_heldIndexLastFrame; }
	bool isDragging() const { return m_isDragging; }
	Optional<int> heldIndex() const { return m_heldIndex; }
	Vec2 currentPos() const { return m_currentPos; }

	RectF getDraggingRect(double cardWidth, double cardHeight) const
	{
		if (!m_isDragging || !m_heldIndex) return RectF{ 0, 0, 0, 0 };
		return RectF{ Arg::center(m_currentPos - m_offset), cardWidth, cardHeight };
	}
};
