#include "core/SpecialSlot.h"
#include "core/Flag.h"
#include "core/Common.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/SpecialCard.h"


SpecialSlot::SpecialSlot()
{
	m_flag_lower_cards.resize(ste_PlayerMakeNum);
	m_flag_lower_cards[static_cast<int>(ste_Player1)].resize(ste_SpecialCardMakeNum,nullptr);
	m_flag_lower_cards[static_cast<int>(ste_Player2)].resize(ste_SpecialCardMakeNum,nullptr);
	m_specialdeck_lower_cards.resize(ste_PlayerMakeNum);
	m_specialdeck_lower_cards[static_cast<int>(ste_Player1)].resize(ste_SpecialCardMakeNum,nullptr);
	m_specialdeck_lower_cards[static_cast<int>(ste_Player2)].resize(ste_SpecialCardMakeNum,nullptr);

}
void SpecialSlot::placeFlagLowerSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer)
{
	// ste_WeatherTacticCardカテゴリのみを受け入れる
	if (card.getCategory() != ste_WeatherTacticCard)
	{
		return; // WeatherTacticCard以外は配置しない
	}

	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer, card);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_flag_lower_cards[playerIndex][slot] = std::make_shared<SpecialCard>(card);
	}
}
void SpecialSlot::placeSpecialDeckLowerSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer)
{
	// ste_ConspiracyTacticCardカテゴリのみを受け入れる
	if (card.getCategory() != ste_ConspiracyTacticCard)
	{
		return; // ConspiracyTacticCard以外は配置しない
	}

	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer, card);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_specialdeck_lower_cards[playerIndex][slot] = std::make_shared<SpecialCard>(card);
	}
}
int SpecialSlot::checkCardSpace(Player* currentPlayer, const SpecialCard& card)
{
	int playerIndex = currentPlayer->getId();
	for (int i = static_cast<int>(ste_SpecialCardMinNum); i <= static_cast<int>(ste_SpecialCardMaxNum); i++)
	{
		if (card.getCategory() == ste_WeatherTacticCard)
		{
			if (m_flag_lower_cards[playerIndex][i] == nullptr)
			{
				return i; // 空きスロットのインデックスを返す
			}
		}
		else if (card.getCategory() == ste_ConspiracyTacticCard)
		{
			if (m_specialdeck_lower_cards[playerIndex][i] == nullptr)
			{
				return i; // 空きスロットのインデックスを返す
			}
		}
	}

	return static_cast<int>(ste_SlotCard_NonSpace);
}

std::shared_ptr <SpecialCard> SpecialSlot::getCard(int playerIndex, int slotIndex) const
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax) || slotIndex < static_cast<int>(ste_SpecialCardMinNum) || slotIndex > static_cast<int>(ste_SpecialCardMaxNum))
	{
		throw std::out_of_range("Invalid player or slot index");
	}
	// フラグ下のカードを返す（必要に応じてm_specialdeck_lower_cardsも対応可能）
	return m_flag_lower_cards[playerIndex][slotIndex];
}

std::shared_ptr <SpecialCard>* SpecialSlot::getCardData(int playerIndex)
{
	return m_flag_lower_cards[playerIndex].data();
}
void SpecialSlot::slotdraw(GameState& gameState, int currentPlayerId)
{
	// Player 2 (ID 1) - フラグ下のカード
	for (int i = 0; i < m_flag_lower_cards[static_cast<int>(ste_Player2)].size(); ++i)
	{
		const RectF rect = getFlagLowerSpecialCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId);
		auto card = m_flag_lower_cards[static_cast<int>(ste_Player2)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else
		{
			rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
			float thickness = 4.0;
			ColorF lineColor = Palette::White;
			rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
			break;
		}
	}

	// Player 2 (ID 1) - 特殊デッキ下のカード
	for (int i = 0; i < m_specialdeck_lower_cards[static_cast<int>(ste_Player2)].size(); ++i)
	{
		const RectF rect = getSpecialDeckLowerSpecialCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId);
		auto card = m_specialdeck_lower_cards[static_cast<int>(ste_Player2)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else
		{
			rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
			float thickness = 4.0;
			ColorF lineColor = Palette::White;
			rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
			break;
		}
	}

	// Player 1 (ID 0) - フラグ下のカード
	for (int i = 0; i < m_flag_lower_cards[static_cast<int>(ste_Player1)].size(); ++i)
	{
		const RectF rect = getFlagLowerSpecialCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId);
		auto card = m_flag_lower_cards[static_cast<int>(ste_Player1)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else
		{
			rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
			float thickness = 4.0;
			ColorF lineColor = Palette::White;
			rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
			break;
		}
	}

	// Player 1 (ID 0) - 特殊デッキ下のカード
	for (int i = 0; i < m_specialdeck_lower_cards[static_cast<int>(ste_Player1)].size(); ++i)
	{
		const RectF rect = getSpecialDeckLowerSpecialCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId);
		auto card = m_specialdeck_lower_cards[static_cast<int>(ste_Player1)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else
		{
			rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
			float thickness = 4.0;
			ColorF lineColor = Palette::White;
			rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
			rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
			break;
		}
	}
}

RectF SpecialSlot::getFlagLowerSpecialCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId)
{

	RectF rect;
	// Get the Flag object using GameState and m_flagIndex
	Flag& currentFlag = gameState.getFlag(m_flagIndex);

	// Current player's slots (bottom)
	if (playerIndex == currentPlayerId)
	{
		rect = { m_flag_init_position.x - (m_specialcard_slot_size.x / 2), currentFlag.getTexture().height() / 2 + m_flag_init_position.y + 20 + (slotIndex+3) * (m_specialcard_slot_size.y / 3), m_specialcard_slot_size.x, m_specialcard_slot_size.y };
	}
	// Opponent's slots (top)
	else
	{
		rect = { m_flag_init_position.x - (m_specialcard_slot_size.x / 2), m_flag_init_position.y - (currentFlag.getTexture().height() / 2) - m_specialcard_slot_size.y - 20 - ((slotIndex+3) * (m_specialcard_slot_size.y / 3)), m_specialcard_slot_size.x, m_specialcard_slot_size.y };
	}
	return rect;
}
RectF SpecialSlot::getSpecialDeckLowerSpecialCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId)
{
	RectF rect;
	// 特殊デッキの位置を取得
	RectF specialdeck_rect = gameState.getSpecialDeck()->getRect();

	// 特殊デッキの下にカードを配置
	// Current player's slots (bottom)
	if (playerIndex == currentPlayerId)
	{
		rect = { specialdeck_rect.x, specialdeck_rect.y + specialdeck_rect.h + 20 + slotIndex * (m_specialcard_slot_size.y / 3), m_specialcard_slot_size.x, m_specialcard_slot_size.y };
	}
	// Opponent's slots (top)
	else
	{
		rect = { specialdeck_rect.x, specialdeck_rect.y - m_specialcard_slot_size.y - 20 - (slotIndex * (m_specialcard_slot_size.y / 3)), m_specialcard_slot_size.x, m_specialcard_slot_size.y };
	}
	return rect;
}
