#include "core/WeatherSlot.h"
#include "core/Flag.h"
#include "core/Common.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/SpecialCard.h"

WeatherSlot::WeatherSlot()
{
	m_weather_cards.resize(ste_PlayerMakeNum);
	m_weather_cards[static_cast<int>(ste_Player1)].resize(ste_SpecialCardMakeNum, nullptr);
	m_weather_cards[static_cast<int>(ste_Player2)].resize(ste_SpecialCardMakeNum, nullptr);
}

void WeatherSlot::placeWeatherCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer)
{
	// ste_WeatherTacticCardカテゴリのみを受け入れる
	if (card.getCategory() != ste_WeatherTacticCard)
	{
		return; // WeatherTacticCard以外は配置しない
	}

	int playerIndex = currentPlayer->getId();

	int slot = checkCardSpace(currentPlayer);
	if (slot != static_cast<int>(ste_SlotCard_NonSpace))
	{
		m_weather_cards[playerIndex][slot] = std::make_shared<SpecialCard>(card);

		// WeatherCardの効果でフラグの勝敗が変わる可能性があるため、両プレイヤーの役とフラグの勝敗を再計算
		Flag& currentFlag = gameState.getFlag(m_flagIndex);

		// 両プレイヤーの役を再計算（カードが揃っている場合のみ）
		currentFlag.checkRoleStatus(gameState, gameState.getPlayer1());
		currentFlag.checkRoleStatus(gameState, gameState.getPlayer2());

		// フラグの勝敗を再計算
		currentFlag.checkFlagStatus(gameState);
	}
}

int WeatherSlot::checkCardSpace(Player* currentPlayer) const
{
	int playerIndex = currentPlayer->getId();
	for (int i = static_cast<int>(ste_SpecialCardMinNum); i <= static_cast<int>(ste_SpecialCardMaxNum); i++)
	{
		if (m_weather_cards[playerIndex][i] == nullptr)
		{
			return i; // 空きスロットのインデックスを返す
		}
	}

	return static_cast<int>(ste_SlotCard_NonSpace);
}

std::shared_ptr<SpecialCard> WeatherSlot::getCard(int playerIndex, int slotIndex) const
{
	if (playerIndex < static_cast<int>(ste_PlayerMin) || playerIndex > static_cast<int>(ste_PlayerMax) || slotIndex < static_cast<int>(ste_SpecialCardMinNum) || slotIndex > static_cast<int>(ste_SpecialCardMaxNum))
	{
		throw std::out_of_range("Invalid player or slot index");
	}
	return m_weather_cards[playerIndex][slotIndex];
}

std::shared_ptr<SpecialCard>* WeatherSlot::getCardData(int playerIndex)
{
	return m_weather_cards[playerIndex].data();
}

void WeatherSlot::slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots)
{
	// Player 2 (ID 1)
	for (int i = 0; i < m_weather_cards[static_cast<int>(ste_Player2)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player2), i, currentPlayerId);
		auto card = m_weather_cards[static_cast<int>(ste_Player2)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else if (showEmptySlots && gameState.getCurrentPlayer()->getId()==ste_Player2)
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
	for (int i = 0; i < m_weather_cards[static_cast<int>(ste_Player1)].size(); ++i)
	{
		const RectF rect = getCardSlotRect(gameState, static_cast<int>(ste_Player1), i, currentPlayerId);
		auto card = m_weather_cards[static_cast<int>(ste_Player1)][i];

		if (card != nullptr)
		{
			card->setRect(rect);
			card->draw();
		}
		else if (showEmptySlots && gameState.getCurrentPlayer()->getId() == ste_Player1)
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

RectF WeatherSlot::getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId)
{
	RectF rect;
	Flag& currentFlag = gameState.getFlag(m_flagIndex);

	// Current player's slots (bottom)
	if (playerIndex == currentPlayerId)
	{
		rect = { m_flag_init_position.x - (m_card_slot_size.x / 2), currentFlag.getTexture().height() / 2 + m_flag_init_position.y + 20 + (slotIndex + 3) * (m_card_slot_size.y / 3), m_card_slot_size.x, m_card_slot_size.y };
	}
	// Opponent's slots (top)
	else
	{
		rect = { m_flag_init_position.x - (m_card_slot_size.x / 2), m_flag_init_position.y - (currentFlag.getTexture().height() / 2) - m_card_slot_size.y - 20 - ((slotIndex + 3) * (m_card_slot_size.y / 3)), m_card_slot_size.x, m_card_slot_size.y };
	}
	return rect;
}

bool WeatherSlot::hasFogCard() const
{
	for (int playerIndex = static_cast<int>(ste_Player1); playerIndex <= static_cast<int>(ste_Player2); ++playerIndex)
	{
		for (const auto& card : m_weather_cards[playerIndex])
		{
			if (card != nullptr && card->getType() == ste_FogCard)
			{
				return true;
			}
		}
	}
	return false;
}

bool WeatherSlot::hasMudCard() const
{
	for (int playerIndex = static_cast<int>(ste_Player1); playerIndex <= static_cast<int>(ste_Player2); ++playerIndex)
	{
		for (const auto& card : m_weather_cards[playerIndex])
		{
			if (card != nullptr && card->getType() == ste_MudCard)
			{
				return true;
			}
		}
	}
	return false;
}
