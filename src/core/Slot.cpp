#include "core/Slot.h"
#include "core/Flag.h"
#include "core/Common.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/SpecialCard.h"
#include "core/WeatherSlot.h"


Slot::Slot()
{
	m_cards.resize(ste_PlayerMakeNum);
	m_cards[static_cast<int>(ste_Player1)].resize(ste_SlotCardMakeNum, nullptr);
	m_cards[static_cast<int>(ste_Player2)].resize(ste_SlotCardMakeNum, nullptr);

}
void Slot::placeCard(GameState& gameState, const Card& card, Player* currentPlayer)
{
	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer, gameState);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_cards[playerIndex][slot] = std::make_shared<Card>(card);

        // GameState と m_flagIndex を使用して Flag オブジェクトを取得
        Flag& currentFlag = gameState.getFlag(m_flagIndex);

		currentFlag.checkRoleStatus(gameState, currentPlayer); // gameState を渡す
		currentFlag.checkFlagStatus(gameState); // gameState を渡す
	}
}

void Slot::placeSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer)
{
	// ste_TroopCardカテゴリのみを受け入れる
	if (card.getCategory() != ste_TroopCard)
	{
		return; // TroopCard以外は配置しない
	}

	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer, gameState);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_cards[playerIndex][slot] = std::make_shared<SpecialCard>(card);

		// SpecialCardの効果を発動
		auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_cards[playerIndex][slot]);
		if (specialCardPtr)
		{
			specialCardPtr->doEffect(&gameState, this);
		}

        // GameState と m_flagIndex を使用して Flag オブジェクトを取得
        Flag& currentFlag = gameState.getFlag(m_flagIndex);

		currentFlag.checkRoleStatus(gameState, currentPlayer); // gameState を渡す
		currentFlag.checkFlagStatus(gameState); // gameState を渡す
	}
}
int Slot::checkCardSpace(Player* currentPlayer, GameState& gameState)
{
	int playerIndex = currentPlayer->getId();

	// 泥カードがある場合は4枚まで、そうでない場合は3枚まで
	WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_flagIndex);
	int maxCards = weatherSlot.hasMudCard() ? static_cast<int>(ste_SlotCardMaxNum) : static_cast<int>(ste_SlotCardMaxNum) - 1;

	for (int i = static_cast<int>(ste_SlotCardMinNum); i <= maxCards; i++)
	{
		if (m_cards[playerIndex][i] == nullptr)
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
std::shared_ptr<CardBase> Slot::getCard(int playerIndex, int slotIndex) const
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax) || slotIndex < static_cast<int>(ste_SlotCardMinNum) || slotIndex > static_cast<int>(ste_SlotCardMaxNum))
	{
		throw std::out_of_range("Invalid player or slot index");
	}
	return m_cards[playerIndex][slotIndex];
}
std::shared_ptr<CardBase>* Slot::getCardData(int playerIndex)
{
	return m_cards[playerIndex].data();
}
void Slot::slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots)
{
	// 泥カードがある場合は4枚まで、そうでない場合は3枚まで表示
	WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_flagIndex);
	int maxCards = weatherSlot.hasMudCard() ? static_cast<int>(ste_SlotCardMakeNum) : static_cast<int>(ste_SlotCardMakeNum) - 1;

	// Player 2 (ID 1)
	for (int i = 0; i < maxCards; ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId); // gameState を渡す
		auto card = m_cards[static_cast<int>(ste_Player2)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else if (showEmptySlots && gameState.getCurrentPlayer()->getId()== ste_Player2)
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
	for (int i = 0; i < maxCards; ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId); // gameState を渡す
		auto card = m_cards[static_cast<int>(ste_Player1)][i];

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
