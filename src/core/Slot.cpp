#include "core/Slot.h"
#include "core/Flag.h"
#include "core/Common.h" // 追加
#include "core/Card.h"
#include "core/Player.h"
#include "core/GameState.h"


Slot::Slot()
{
	m_cards.resize(ste_PlayerMakeNum);
	m_cards[static_cast<int>(ste_Player1)].resize(ste_SlotCardMakeNum);
	m_cards[static_cast<int>(ste_Player2)].resize(ste_SlotCardMakeNum);

};
void Slot::placeCard(GameState& gameState, const Card& card, Player* currentPlayer) // シグネチャ更新
{
	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		Card cardToPlace = card;
		m_cards[playerIndex][slot] = cardToPlace;

        // GameState と m_flagIndex を使用して Flag オブジェクトを取得
        Flag& currentFlag = gameState.getFlag(m_flagIndex);

		currentFlag.checkRoleStatus(gameState, currentPlayer); // gameState を渡す
		currentFlag.checkFlagStatus(gameState); // gameState を渡す
	}
}
int Slot::checkCardSpace(Player* currentPlayer)
{
	int playerIndex = currentPlayer->getId();
	for (int i = static_cast<int>(ste_SlotCardMinNum); i <= static_cast<int>(ste_SlotCardMaxNum); i++)
	{
		if (m_cards[playerIndex][i].getValue() == static_cast<int>(ste_NoneCard))
		{
			return i; // 空きスロットのインデックスを返す
		}
	}

	return static_cast<int>(ste_SlotCard_NonSpace);
}
void Slot::setCardSpace(bool isEmpty, int playerIndex)
{
	m_card_space[playerIndex] = isEmpty;
}
bool Slot::getCardSpace(int playerIndex)
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax))
	{
		throw std::out_of_range("Invalid player index");
	}
	return m_card_space[playerIndex];
}
Card Slot::getCard(int playerIndex, int slotIndex) const
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax) || slotIndex < static_cast<int>(ste_SlotCardMinNum) || slotIndex > static_cast<int>(ste_SlotCardMaxNum))
	{
		throw std::out_of_range("Invalid player or slot index");
	}
	return m_cards[playerIndex][slotIndex];
}
Card* Slot::getCardData(int playerIndex)
{
	return m_cards[playerIndex].data();
}
void Slot::slotdraw(GameState& gameState, int currentPlayerId)
{
	// Player 2 (ID 1)
	for (int i = 0; i < m_cards[static_cast<int>(ste_Player2)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId); // gameState を渡す
		Card card = m_cards[static_cast<int>(ste_Player2)][i];

		if (card.getValue() != static_cast<int>(ste_NoneCard))
		{
			card.setRect(rect);
			card.draw();
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

	// Player 1 (ID 0)
	for (int i = 0; i < m_cards[static_cast<int>(ste_Player1)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId); // gameState を渡す
		Card card = m_cards[static_cast<int>(ste_Player1)][i];

		if (card.getValue() != static_cast<int>(ste_NoneCard))
		{
			card.setRect(rect);
			card.draw();
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
RectF Slot::getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId)
{

	RectF rect;
	// Get the Flag object using GameState and m_flagIndex
    Flag& currentFlag = gameState.getFlag(m_flagIndex);

	// Current player's slots (bottom)
	if (playerIndex == currentPlayerId)
	{
		rect = { m_flag_init_position.x - (m_card_slot_size.x / 2), currentFlag.getTexture().height() / 2 + m_flag_init_position.y + 20 + slotIndex * (m_card_slot_size.y / 3), m_card_slot_size.x, m_card_slot_size.y};
	}
	// Opponent's slots (top)
	else
	{
		rect = { m_flag_init_position.x - (m_card_slot_size.x / 2), m_flag_init_position.y - (currentFlag.getTexture().height() / 2) - m_card_slot_size.y - 20 - (slotIndex * (m_card_slot_size.y / 3)), m_card_slot_size.x, m_card_slot_size.y };
	}
	return rect;
}
