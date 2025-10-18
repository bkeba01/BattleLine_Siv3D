#include "core/Deck.h"
#include <algorithm>
#include <random>
#include "core/Common.h"

Deck::Deck(const Font& font, const Texture& texture)
    : m_font(font), m_texture(texture)
{
    m_cards.reserve(60);
    for (int c = ste_ColorMinValue; c <= ste_ColorMaxValue; ++c)
    {
        for (int v = ste_CardMinValue; v <= ste_CardMaxValue; ++v) 
        {
            m_cards.emplace_back(c, v, m_font, m_texture);
        }
    }
}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}

Card Deck::drawCard() 
{
    if (m_cards.empty()) 
    {
        throw std::out_of_range("Deck is empty");
    }

    Card drawnCard = m_cards.back();
    m_cards.pop_back();
    return drawnCard;
}

bool Deck::isEmpty() const {
    return m_cards.empty();
}
