#ifndef CARD_H
#define CARD_H
#include <Siv3D.hpp>
class Card {
    private:
        int m_color;
        int m_value;
		Font m_font;
		Texture m_texture;
		Vec2 m_card_hand_size;
		Vec2 m_card_hand_space;
		RectF m_rect;
    public:
        Card();
        Card(int c, int v,Font font,Texture texture);
        int getColor() const;
        int getValue() const;
		void setFont(Font);
		Font getFont();
		void setTexture(Texture);
		void setRect(RectF rect) { m_rect = rect; };
		RectF getRect() {return m_rect; };
		void setCardHandSize(Vec2 card_hand_size) { m_card_hand_size = card_hand_size; };
		void setCardHandSpace(Vec2 card_hand_space) { m_card_hand_space = card_hand_space; };
		Vec2 getCardHandSize() { return m_card_hand_size; };
		Vec2 getCardHandSpace() { return m_card_hand_space; };
		Texture getTexture();
        //戻り値の型 operator記号(引数) const {}
        //operatorで記号を打つことで、その記号を使った処理が行える
        //今回は==を使って、カードの色と値が同じかどうかを比較できるようにする
        bool operator==(const Card& other) const {
            return m_color == other.m_color && m_value == other.m_value;
        }
		void draw() const;
		void drawBack() const;
};

#endif
