#include "core/Deck.h"
#include <algorithm>
#include <random>
#include "core/Common.h"

Deck::Deck(const Font& font, const Texture& texture,const Texture& backtexture)
    : m_font(font), m_texture(texture),m_back_texture(backtexture)
{
    m_cards.reserve(60);
    for (int c = ste_ColorMinValue; c <= ste_ColorMaxValue; ++c)
    {
        for (int v = ste_CardMinValue; v <= ste_CardMaxValue; ++v) 
        {
            m_cards.emplace_back(c, v, m_font, texture,backtexture);
        }
    }
}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}

void Deck::shuffleWithSeed(uint32_t seed) {
    std::mt19937 g(seed);
    std::shuffle(m_cards.begin(), m_cards.end(), g);
    std::cout << "[Deck] Shuffled with seed: " << seed << std::endl;
}

std::optional<Card> Deck::drawCard()
{
    if (m_cards.empty())
    {
        return std::nullopt;
    }

    Card drawnCard = m_cards.back();
    m_cards.pop_back();
    return drawnCard;
}

std::shared_ptr<Card> Deck::draw()
{
    if (m_cards.empty())
    {
        return nullptr;
    }

    Card drawnCard = m_cards.back();
    m_cards.pop_back();
    return std::make_shared<Card>(drawnCard);
}

void Deck::returnCard(std::shared_ptr<CardBase> card)
{
    // CardBaseからCardにダウンキャスト
    auto cardPtr = std::dynamic_pointer_cast<Card>(card);
    if (cardPtr)
    {
        // 山札の一番上（最後）に追加（drawCard()がbackから引くため）
        m_cards.push_back(*cardPtr);
    }
}

std::shared_ptr<Card> Deck::removeCard(size_t index)
{
    if (index >= m_cards.size())
    {
        return nullptr;
    }
    Card removedCard = m_cards[index];
    m_cards.erase(m_cards.begin() + index);
    return std::make_shared<Card>(removedCard);
}

bool Deck::isEmpty() const {
    return m_cards.empty();
}

void Deck::drawDeck() const
{
	m_rect.draw(Arg::top = ColorF{ 0.5, 0.7, 0.9 }, Arg::bottom = ColorF{ 0.5, 0.9, 0.7 });
	m_rect.drawFrame(2, 0, Palette::Black);
	m_back_texture.resized(45).drawAt(m_rect.center().movedBy(0, 0));
}

// デッキの順序をシリアル化（カードIDの配列として返す）
s3d::Array<int32> Deck::serializeDeck() const
{
	s3d::Array<int32> cardIds;
	cardIds.reserve(m_cards.size());

	for (const auto& card : m_cards)
	{
		// カードID = color * 100 + value
		int32 cardId = card.getColor() * 100 + card.getValue();
		cardIds.push_back(cardId);
	}

	std::cout << "[Deck] Serialized " << cardIds.size() << " cards" << std::endl;
	return cardIds;
}

// デッキの順序を復元（カードIDの配列から再構築）
void Deck::deserializeDeck(const s3d::Array<int32>& cardIds)
{
	std::cout << "[Deck] Deserializing " << cardIds.size() << " cards" << std::endl;

	// 既存のデッキをクリア
	m_cards.clear();
	m_cards.reserve(cardIds.size());

	// カードIDから順番にカードを再構築
	for (int32 cardId : cardIds)
	{
		int color = cardId / 100;
		int value = cardId % 100;
		m_cards.emplace_back(color, value, m_font, m_texture, m_back_texture);
	}

	std::cout << "[Deck] Deserialized " << m_cards.size() << " cards successfully" << std::endl;
}
