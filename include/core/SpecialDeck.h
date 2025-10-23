#ifndef SPECIALDECK_H
#define SPECIALDECK_H

#include <vector>
#include <optional>
#include "SpecialCard.h"
#include <Siv3D.hpp>

class SpecialDeck {
    private:
        std::vector<SpecialCard> m_cards;
        Font m_font;
        RectF m_rect;
        Texture m_texture;
        Texture m_back_texture;

    public:
        SpecialDeck(const Font& font, const Texture& texture, const Texture& backtexture,const Font& smallfont);
        void shuffle();
        std::optional<SpecialCard> drawCard();
        bool isEmpty() const;
        void setRect(RectF rect) { m_rect = rect; }
        RectF getRect() const { return m_rect; }
        void drawDeck() const;
};

#endif
