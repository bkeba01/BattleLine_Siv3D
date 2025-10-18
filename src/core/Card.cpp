#include "core/Card.h"

Card::Card() : m_color(0), m_value(0) {}
Card::Card(int c, int v,Font font,Texture texture) : m_color(c), m_value(v),m_font(font),m_texture(texture) {}
int Card::getColor() const { return m_color; }
int Card::getValue() const { return m_value; }

void Card::draw(const RectF& rect) const
{
	if (getValue() == 0)
	{
		rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
		return;
	}

	const HSV cardColor = HSV{ static_cast<double>(getColor() * 60 - 60), 0.6, 0.95 };
	rect.draw(cardColor);
	rect.drawFrame(2, 0, Palette::Black);
	//font(getValue()).drawAt(rect.center().movedBy(0, -10), Palette::Black);
	m_font(getValue()).draw(
		rect.x + 5,            // 左端から5px右
		rect.y + 5,            // 上端から5px下
		Palette::Black
	);
	double textWidth = m_font(getValue()).region().w;   // 文字の幅
	double textHeight = m_font(getValue()).region().h;   // 文字の高さ
	m_font(getValue()).draw(
		rect.x + rect.w - textWidth - 5,  // 右端から5px左
		rect.y + rect.h - textHeight - 5, // 下端から5px上
		Palette::Black
	);
	m_texture.resized(30).drawAt(rect.center().movedBy(0, 15));
}

void Card::drawBack(const RectF& rect) const
{
	rect.draw(Palette::Darkblue);
	rect.drawFrame(2, 0, Palette::Black);
}

void Card::setFont(Font font)
{
	m_font = font;
}

Font Card::getFont()
{
	return m_font;
}

void Card::setTexture(Texture texture)
{
	m_texture = texture;
}

Texture Card::getTexture()
{
	return m_texture;
}
