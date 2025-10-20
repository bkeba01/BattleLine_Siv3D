#ifndef DECK_H
#define DECK_H

#include <vector>
#include "Card.h"
#include <siv3D.hpp>

class Deck {
    private:
        std::vector<Card> m_cards;
        Font m_font;
		RectF m_rect;
		Texture m_texture;
		Texture m_back_texture;
    public:
        Deck(const Font& font, const Texture& texture,const Texture& backtexture);
        void shuffle();
        Card drawCard();
        bool isEmpty() const;
		void setRect(RectF rect) { m_rect = rect; };
		RectF getRect() { return m_rect; };
		void drawDeck() const;
};

#endif
