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
        Font m_small_font;
        RectF m_rect;
        Texture m_texture;
        Texture m_back_texture;

    public:
        SpecialDeck(const Font& font, const Texture& texture, const Texture& backtexture,const Font& smallfont);
        void shuffle();
        void shuffleWithSeed(uint32_t seed); // ネットワーク同期用
        std::optional<SpecialCard> drawCard();
        std::shared_ptr<SpecialCard> draw(); // ReconCard用
        void returnCard(std::shared_ptr<CardBase> card); // ReconCard用
        const std::vector<SpecialCard>& getCards() const { return m_cards; } // ReconCard一覧表示用
        std::shared_ptr<SpecialCard> removeCard(size_t index); // ReconCard: 指定インデックスのカードを削除
        bool isEmpty() const;

        // デッキ同期用メソッド
        s3d::Array<int32> serializeDeck() const; // デッキの順序をシリアル化
        void deserializeDeck(const s3d::Array<int32>& cardIds); // デッキの順序を復元
        void setRect(RectF rect) { m_rect = rect; }
        RectF getRect() const { return m_rect; }
        void drawDeck(bool canUseSpecialCard = true) const;
};

#endif
