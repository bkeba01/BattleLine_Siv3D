#ifndef DECK_H
#define DECK_H

#include <vector>
#include "Card.h"

class Deck {
    private:
        std::vector<Card> m_cards;
        Font m_font;
        Texture m_texture;
    public:
        Deck(const Font& font, const Texture& texture);
        void shuffle();
        Card drawCard();
        bool isEmpty() const;
};

#endif
