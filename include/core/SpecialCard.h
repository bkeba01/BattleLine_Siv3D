#ifndef SPECIALCARD_H
#define SPECIALCARD_H
#include <Siv3D.hpp>
#include "core/CardBase.h"
#include "core/Common.h"

class GameState;
class Slot;

class SpecialCard : public CardBase
{
	private:
		SpecialCardType m_type;
		SpecialCardCategory m_category;
		String m_name;
		String m_description;
		Font m_smallFont;
		mutable int m_value;
		mutable int m_color;
	public:
		SpecialCard();
		SpecialCard(SpecialCardType type, SpecialCardCategory category, const String& name, const String& description,
						 const Font& font, const Texture& texture, const Texture& backtexture, const Font& smallfont);
		void draw()const override;
		void draw(const RectF& rect) const override;
		void drawBack() const override;
		SpecialCardType getType() const { return m_type; };
		SpecialCardCategory getCategory() const { return m_category; };
		String getName() const { return m_name; };
		String getDescription() const { return m_description; };
		void doEffect(GameState* gameState, Slot* currntSlot) const;
		int getValue() const { return m_value; };
		int getColor() const { return m_color; };
		void setValue(int value) { m_value = value; };
		void setColor(int color) { m_color = color; };

		// ネットワーク同期用のカードID (特殊カードはタイプをそのまま返す)
		int getId() const override {
			return static_cast<int>(m_type);
		}
};
#endif
