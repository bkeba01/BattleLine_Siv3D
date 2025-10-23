#include "core/CardBase.h"

CardBase::CardBase()
    : m_font()
    , m_texture()
    , m_back_texture()
    , m_card_hand_size(0, 0)
    , m_card_hand_space(0, 0)
    , m_rect()
{
}

CardBase::CardBase(const Font& font, const Texture& texture, const Texture& backtexture)
    : m_font(font)
    , m_texture(texture)
    , m_back_texture(backtexture)
    , m_card_hand_size(0, 0)
    , m_card_hand_space(0, 0)
    , m_rect()
{
}

void CardBase::setRect(RectF rect)
{
    m_rect = rect;
}

RectF CardBase::getRect() const
{
    return m_rect;
}

void CardBase::setCardHandSize(Vec2 size)
{
    m_card_hand_size = size;
}

void CardBase::setCardHandSpace(Vec2 space)
{
    m_card_hand_space = space;
}

Vec2 CardBase::getCardHandSize() const
{
    return m_card_hand_size;
}

Vec2 CardBase::getCardHandSpace() const
{
    return m_card_hand_space;
}

void CardBase::setFont(Font font)
{
    m_font = font;
}

Font CardBase::getFont() const
{
    return m_font;
}

void CardBase::setTexture(Texture texture)
{
    m_texture = texture;
}

Texture CardBase::getTexture() const
{
    return m_texture;
}

void CardBase::setBackTexture(Texture texture)
{
    m_back_texture = texture;
}

Texture CardBase::getBackTexture() const
{
    return m_back_texture;
}
