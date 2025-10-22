#include "core/SpecialDeck.h"
#include <algorithm>
#include <random>
#include "core/Common.h"

SpecialDeck::SpecialDeck(const Font& font, const Texture& texture, const Texture& backtexture)
    : m_font(font), m_texture(texture), m_back_texture(backtexture)
{
    m_cards.reserve(ste_SpecialDeckSize);

    // 特殊カードの種類ごとに枚数を設定
    // ワイルドカード 3枚
    for (int i = 0; i < 3; ++i) {
        m_cards.emplace_back(ste_Wildcard, U"ワイルド", U"任意の色・値として使える", font, texture, backtexture);
    }
    // スティールカード 2枚
    for (int i = 0; i < 2; ++i) {
        m_cards.emplace_back(ste_StealCard, U"スティール", U"相手のカードを1枚奪う", font, texture, backtexture);
    }
    // デストロイカード 2枚
    for (int i = 0; i < 2; ++i) {
        m_cards.emplace_back(ste_DestroyCard, U"デストロイ", U"相手のカードを1枚破壊", font, texture, backtexture);
    }
    // スワップカード 2枚
    for (int i = 0; i < 2; ++i) {
        m_cards.emplace_back(ste_SwapCard, U"スワップ", U"カードを入れ替える", font, texture, backtexture);
    }
    // ダブルバリュー 1枚
    m_cards.emplace_back(ste_DoubleValue, U"ダブル", U"値を2倍にする", font, texture, backtexture);
}

void SpecialDeck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}

std::optional<SpecialCard> SpecialDeck::drawCard()
{
    if (m_cards.empty())
    {
        return std::nullopt;
    }

    SpecialCard drawnCard = m_cards.back();
    m_cards.pop_back();
    return drawnCard;
}

bool SpecialDeck::isEmpty() const {
    return m_cards.empty();
}

void SpecialDeck::drawDeck() const
{
    // 特殊デッキ用の描画（金色のグラデーション）
    m_rect.draw(Arg::top = ColorF{ 0.9, 0.8, 0.3 }, Arg::bottom = ColorF{ 0.8, 0.6, 0.2 });
    m_rect.drawFrame(4, 0, Palette::Gold);
    m_back_texture.resized(45).drawAt(m_rect.center().movedBy(0, 0));
}
