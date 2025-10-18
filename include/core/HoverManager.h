#pragma once

class HoverManager
{
protected:
	Optional<int> m_hoveredIndex = none;

public:
	HoverManager() = default;
	virtual ~HoverManager() = default;

	virtual void updateHover(const Array<RectF>& cardRects)
	{
		m_hoveredIndex = none;

		for (int i = 0; i < static_cast<int>(cardRects.size()); ++i)
		{
			if (cardRects[i].mouseOver())
			{
				m_hoveredIndex = i;
				break;
			}
		}
	}

	Optional<int> hoveredIndex() const { return m_hoveredIndex; }
};
