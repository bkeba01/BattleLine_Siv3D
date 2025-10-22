#ifndef SPECIALCARD_H
#define SPECIALCARD_H
#include <Siv3D.hpp>
#include "core/Common.h"

class SpecialCard {
    private:
        SpecialCardType m_type;
        String m_name;
        String m_description;
        Font m_font;
        Texture m_texture;
        Texture m_back_texture;
        Vec2 m_card_hand_size;
        Vec2 m_card_hand_space;
        RectF m_rect;

    public:
        SpecialCard();
        SpecialCard(SpecialCardType type, const String& name, const String& description,
                    const Font& font, const Texture& texture, const Texture& backtexture);

        SpecialCardType getType() const { return m_type; }
        String getName() const { return m_name; }
        String getDescription() const { return m_description; }

        void setRect(RectF rect) { m_rect = rect; }
        RectF getRect() const { return m_rect; }

        void setCardHandSize(Vec2 size) { m_card_hand_size = size; }
        void setCardHandSpace(Vec2 space) { m_card_hand_space = space; }
        Vec2 getCardHandSize() const { return m_card_hand_size; }
        Vec2 getCardHandSpace() const { return m_card_hand_space; }

        void setFont(Font font) { m_font = font; }
        Font getFont() const { return m_font; }

        void setTexture(Texture texture) { m_texture = texture; }
        Texture getTexture() const { return m_texture; }

        void setBackTexture(Texture texture) { m_back_texture = texture; }
        Texture getBackTexture() const { return m_back_texture; }

        void draw() const;
        void drawBack() const;

        bool operator==(const SpecialCard& other) const {
            return m_type == other.m_type;
        }
};

#endif
