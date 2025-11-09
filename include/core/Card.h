#ifndef CARD_H
#define CARD_H
#include <Siv3D.hpp>
#include "core/CardBase.h"

class Card : public CardBase {
    private:
        int m_color;
        int m_value;

    public:
        Card();
        Card(int c, int v, Font font, Texture texture, Texture backtexture);
        int getColor() const;
        int getValue() const;

        //戻り値の型 operator記号(引数) const {}
        //operatorで記号を打つことで、その記号を使った処理が行える
        //今回は==を使って、カードの色と値が同じかどうかを比較できるようにする
        bool operator==(const Card& other) const {
            return m_color == other.m_color && m_value == other.m_value;
        }

        // ネットワーク同期用のカードID (color * 100 + value)
        int getId() const override {
            return m_color * 100 + m_value;
        }

		void draw() const override;
		void draw(const RectF& rect) const override;
		void drawBack() const override;
};

#endif
