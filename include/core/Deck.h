#ifndef DECK_H
#define DECK_H

#include <vector>
#include <optional>
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
        void shuffleWithSeed(uint32_t seed); // ネットワーク同期用
        std::optional<Card> drawCard();
        std::shared_ptr<Card> draw(); // ReconCard用
        void returnCard(std::shared_ptr<CardBase> card); // ReconCard用
        const std::vector<Card>& getCards() const { return m_cards; } // ReconCard一覧表示用
        std::shared_ptr<Card> removeCard(size_t index); // ReconCard: 指定インデックスのカードを削除
        bool isEmpty() const;

        // デッキ同期用メソッド
        s3d::Array<int32> serializeDeck() const; // デッキの順序をシリアル化
        void deserializeDeck(const s3d::Array<int32>& cardIds); // デッキの順序を復元
		void setRect(RectF rect) { m_rect = rect; };
		RectF getRect() { return m_rect; };
		void drawDeck() const;
};

#endif
