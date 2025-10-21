#include "core/Card.h"

Card::Card() : m_color(0), m_value(0), m_font(), m_texture(), m_back_texture() {}
Card::Card(int c, int v,Font font,Texture texture,Texture backtexture) : m_color(c), m_value(v),m_font(font),m_texture(texture),m_back_texture(backtexture) {}
int Card::getColor() const { return m_color; }
int Card::getValue() const { return m_value; }

void Card::draw() const
{
	if (getValue() == 0)
	{
		m_rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
		return;
	}

	const HSV cardColor = HSV{ static_cast<double>(getColor() * 60 - 60), 0.6, 0.95 };
	m_rect.draw(cardColor);
	m_rect.drawFrame(2, 0, Palette::Black);
	//font(getValue()).drawAt(rect.center().movedBy(0, -10), Palette::Black);
	m_font(getValue()).draw(
		m_rect.x + 5,            // 左端から5px右
		m_rect.y + 5,            // 上端から5px下
		Palette::Black
	);
	double textWidth = m_font(getValue()).region().w;   // 文字の幅
	double textHeight = m_font(getValue()).region().h;   // 文字の高さ
	m_font(getValue()).draw(
		m_rect.x + m_rect.w - textWidth - 5,  // 右端から5px左
		m_rect.y + m_rect.h - textHeight - 5, // 下端から5px上
		Palette::Black
	);
	m_texture.resized(30).drawAt(m_rect.center().movedBy(0, 15));
}

void Card::drawBack() const
{
	m_rect.draw(Palette::Darkblue);
	m_back_texture.resized(m_rect.w * 0.8, m_rect.h * 0.8).drawAt(m_rect.center());
	m_rect.drawFrame(2, 0, Palette::Black);
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
