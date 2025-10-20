#include "core/Deck.h"
#include <algorithm>
#include <random>
#include "core/Common.h"

Deck::Deck(const Font& font, const Texture& texture,const Texture& backtexture)
    : m_font(font), m_texture(texture),m_back_texture(backtexture)
{
    m_cards.reserve(60);
    for (int c = ste_ColorMinValue; c <= ste_ColorMaxValue; ++c)
    {
        for (int v = ste_CardMinValue; v <= ste_CardMaxValue; ++v) 
        {
            m_cards.emplace_back(c, v, m_font, texture,backtexture);
        }
    }
}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}

std::optional<Card> Deck::drawCard() 
{
    if (m_cards.empty()) 
    {
        return std::nullopt;
    }

    Card drawnCard = m_cards.back();
    m_cards.pop_back();
    return drawnCard;
}

bool Deck::isEmpty() const {
    return m_cards.empty();
}

void Deck::drawDeck() const
{
	m_rect.draw(Arg::top = ColorF{ 0.5, 0.7, 0.9 }, Arg::bottom = ColorF{ 0.5, 0.9, 0.7 });
	m_rect.drawFrame(2, 0, Palette::Black);
	m_back_texture.resized(45).drawAt(m_rect.center().movedBy(0, 0));
}
