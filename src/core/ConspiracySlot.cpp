#include "core/ConspiracySlot.h"
#include "core/Common.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/SpecialCard.h"

ConspiracySlot::ConspiracySlot()
{
	m_conspiracy_cards.resize(ste_PlayerMakeNum);
	m_conspiracy_cards[static_cast<int>(ste_Player1)].resize(ste_SpecialCardMakeNum, nullptr);
	m_conspiracy_cards[static_cast<int>(ste_Player2)].resize(ste_SpecialCardMakeNum, nullptr);
}

void ConspiracySlot::placeConspiracyCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer)
{
	// ste_ConspiracyTacticCardカテゴリのみを受け入れる
	if (card.getCategory() != ste_ConspiracyTacticCard)
	{
		return; // ConspiracyTacticCard以外は配置しない
	}

	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_conspiracy_cards[playerIndex][slot] = std::make_shared<SpecialCard>(card);

		// カードの種類に応じて効果を発動
		switch (card.getType())
		{
		case ste_ReconCard:
			gameState.startReconCard();
			break;
		case ste_DeploymentCard:
			gameState.startDeploymentCard();
			break;
		case ste_EscapeCard:
			// TODO: 実装
			break;
		case ste_BetrayalCard:
			// TODO: 実装
			break;
		default:
			break;
		}
	}
}

int ConspiracySlot::checkCardSpace(Player* currentPlayer) const
{
	int playerIndex = currentPlayer->getId();
	for (int i = static_cast<int>(ste_SpecialCardMinNum); i <= static_cast<int>(ste_SpecialCardMaxNum); i++)
	{
		if (m_conspiracy_cards[playerIndex][i] == nullptr)
		{
			return i; // 空きスロットのインデックスを返す
		}
	}

	return static_cast<int>(ste_SlotCard_NonSpace);
}

std::shared_ptr<SpecialCard> ConspiracySlot::getCard(int playerIndex, int slotIndex) const
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax) || slotIndex < static_cast<int>(ste_SpecialCardMinNum) || slotIndex > static_cast<int>(ste_SpecialCardMaxNum))
	{
		throw std::out_of_range("Invalid player or slot index");
	}
	return m_conspiracy_cards[playerIndex][slotIndex];
}

std::shared_ptr<SpecialCard>* ConspiracySlot::getCardData(int playerIndex)
{
	return m_conspiracy_cards[playerIndex].data();
}

void ConspiracySlot::slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots)
{
	// Player 2 (ID 1)
	for (int i = 0; i < m_conspiracy_cards[static_cast<int>(ste_Player2)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId);
		auto card = m_conspiracy_cards[static_cast<int>(ste_Player2)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else if (showEmptySlots&& gameState.getCurrentPlayer()->getId() == ste_Player2)
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

	// Player 1 (ID 0)
	for (int i = 0; i < m_conspiracy_cards[static_cast<int>(ste_Player1)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId);
		auto card = m_conspiracy_cards[static_cast<int>(ste_Player1)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else if (showEmptySlots&& gameState.getCurrentPlayer()->getId() == ste_Player1)
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

RectF ConspiracySlot::getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId)
{
	RectF rect;
	RectF specialdeck_rect = gameState.getSpecialDeck()->getRect();

	// Current player's slots (below special deck)
	if (playerIndex == currentPlayerId)
	{
		rect = { specialdeck_rect.x, specialdeck_rect.y + specialdeck_rect.h + 20 + slotIndex * (m_card_slot_size.y / 3), m_card_slot_size.x, m_card_slot_size.y };
	}
	// Opponent's slots (above special deck)
	else
	{
		rect = { specialdeck_rect.x, specialdeck_rect.y - m_card_slot_size.y - 20 - (slotIndex * (m_card_slot_size.y / 3)), m_card_slot_size.x, m_card_slot_size.y };
	}
	return rect;
}
