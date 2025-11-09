#include "core/SpecialDeck.h"
#include <algorithm>
#include <random>
#include "core/Common.h"

SpecialDeck::SpecialDeck(const Font& font, const Texture& texture, const Texture& backtexture, const Font& smallfont)
    : m_font(font), m_small_font(smallfont), m_texture(texture), m_back_texture(backtexture)
{
    m_cards.reserve(ste_SpecialDeckSize);

    // 特殊カードの種類ごとに枚数を設定
    // ワイルドカード 3枚（部隊カード）
    for (int i = 0; i < 2; ++i) {
        m_cards.emplace_back(ste_WildCard, ste_TroopCard, U"ワイルド", U"任意の色・値として使える", font, texture, backtexture, smallfont);
    }
	m_cards.emplace_back(ste_WildCard_Eight, ste_TroopCard, U"ワイルド8", U"任意の色で8として使える", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_WildCard_Shield, ste_TroopCard, U"ワイルド盾", U"任意の色で1,2,3として使える", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_FogCard, ste_WeatherTacticCard, U"霧", U"役ではなく合計値を取得", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_MudCard, ste_WeatherTacticCard, U"泥", U"4枚での役判断する", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_ReconCard, ste_ConspiracyTacticCard, U"偵察", U"山札から任意のカードを3枚取得、手札から2枚を山札に返す。", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_DeploymentCard, ste_ConspiracyTacticCard, U"配置展開", U"自分のSlotからカードを選択、そのカードを別のSlot、もしくは削除する。", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_EscapeCard, ste_ConspiracyTacticCard, U"脱走", U"相手のSlotからカードを選択、削除する。", font, texture, backtexture, smallfont);
	m_cards.emplace_back(ste_BetrayalCard, ste_ConspiracyTacticCard, U"裏切り", U"相手のSlotからカードを選択、自分のSlotに配置", font, texture, backtexture, smallfont);
}

void SpecialDeck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}

void SpecialDeck::shuffleWithSeed(uint32_t seed) {
    std::mt19937 g(seed);
    std::shuffle(m_cards.begin(), m_cards.end(), g);
    std::cout << "[SpecialDeck] Shuffled with seed: " << seed << std::endl;
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

std::shared_ptr<SpecialCard> SpecialDeck::draw()
{
    if (m_cards.empty())
    {
        return nullptr;
    }

    SpecialCard drawnCard = m_cards.back();
    m_cards.pop_back();
    return std::make_shared<SpecialCard>(drawnCard);
}

void SpecialDeck::returnCard(std::shared_ptr<CardBase> card)
{
    // CardBaseからSpecialCardにダウンキャスト
    auto cardPtr = std::dynamic_pointer_cast<SpecialCard>(card);
    if (cardPtr)
    {
        // 山札の一番上（最後）に追加（drawCard()がbackから引くため）
        m_cards.push_back(*cardPtr);
    }
}

std::shared_ptr<SpecialCard> SpecialDeck::removeCard(size_t index)
{
    if (index >= m_cards.size())
    {
        return nullptr;
    }
    SpecialCard removedCard = m_cards[index];
    m_cards.erase(m_cards.begin() + index);
    return std::make_shared<SpecialCard>(removedCard);
}

bool SpecialDeck::isEmpty() const {
    return m_cards.empty();
}

void SpecialDeck::drawDeck(bool canUseSpecialCard) const
{
    // 特殊デッキ用の描画（金色のグラデーション）
    m_rect.draw(Arg::top = ColorF{ 0.9, 0.8, 0.3 }, Arg::bottom = ColorF{ 0.8, 0.6, 0.2 });

    // 枠線の色を使用可否に応じて変更
    ColorF frameColor = canUseSpecialCard ? Palette::Lime : Palette::Gray;
    m_rect.drawFrame(4, 0, frameColor);

    m_back_texture.resized(45).drawAt(m_rect.center().movedBy(0, 0));
}

// デッキの順序をシリアル化（カードIDの配列として返す）
s3d::Array<int32> SpecialDeck::serializeDeck() const
{
	s3d::Array<int32> cardIds;
	cardIds.reserve(m_cards.size());

	for (const auto& card : m_cards)
	{
		// SpecialCardはタイプをそのままIDとして使用
		int32 cardId = static_cast<int32>(card.getType());
		cardIds.push_back(cardId);
	}

	std::cout << "[SpecialDeck] Serialized " << cardIds.size() << " cards" << std::endl;
	return cardIds;
}

// デッキの順序を復元（カードIDの配列から再構築）
void SpecialDeck::deserializeDeck(const s3d::Array<int32>& cardIds)
{
	std::cout << "[SpecialDeck] Deserializing " << cardIds.size() << " cards" << std::endl;

	// 既存のデッキをクリア
	m_cards.clear();
	m_cards.reserve(cardIds.size());

	// カードIDから順番にカードを再構築
	// 注: SpecialCardのコンストラクタは特殊なため、元のデッキと同じ順序で再構築する必要がある
	// ここでは、cardIdsの順序に従って、元のSpecialCardを再配置する
	// そのため、一時的な完全なデッキを作成してから並べ替える

	// まず、全種類のSpecialCardを作成（元のコンストラクタと同じ）
	SpecialDeck tempDeck(m_font, m_texture, m_back_texture, m_small_font);

	// cardIdsの順序に従って、tempDeckからカードをコピー
	for (int32 cardId : cardIds)
	{
		SpecialCardType targetType = static_cast<SpecialCardType>(cardId);

		// tempDeckから該当するカードを探してコピー
		for (const auto& card : tempDeck.m_cards)
		{
			if (card.getType() == targetType)
			{
				m_cards.push_back(card);
				break;
			}
		}
	}

	std::cout << "[SpecialDeck] Deserialized " << m_cards.size() << " cards successfully" << std::endl;
}
