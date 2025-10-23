#ifndef CARDBASE_H
#define CARDBASE_H
#include <Siv3D.hpp>

class CardBase {
protected:
    Font m_font;
    Texture m_texture;
    Texture m_back_texture;
    Vec2 m_card_hand_size;
    Vec2 m_card_hand_space;
    RectF m_rect;

public:
	CardBase();
	CardBase(const Font& font, const Texture& texture, const Texture& backtexture);
    virtual ~CardBase() = default;

    // 共通のセッター・ゲッター
    virtual void setRect(RectF rect);
    virtual RectF getRect() const;

    virtual void setCardHandSize(Vec2 size);
    virtual void setCardHandSpace(Vec2 space);
    virtual Vec2 getCardHandSize() const;
    virtual Vec2 getCardHandSpace() const;

    virtual void setFont(Font font);
    virtual Font getFont() const;

    virtual void setTexture(Texture texture);
    virtual Texture getTexture() const;

    virtual void setBackTexture(Texture texture);
    virtual Texture getBackTexture() const;

    // 純粋仮想関数（派生クラスで実装必須）
    virtual void draw() const = 0;
    virtual void drawBack() const = 0;
};

#endif
