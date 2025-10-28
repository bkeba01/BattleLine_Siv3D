#ifndef WEATHERSLOT_H
#define WEATHERSLOT_H

#include <Siv3D.hpp>
#include <vector>
#include <memory>
#include "core/Common.h"

class Player;
class Card;
class Flag;
class GameState;
class SpecialCard;

class WeatherSlot
{
private:
	std::vector<std::vector<std::shared_ptr<SpecialCard>>> m_weather_cards;
	Vec2 m_card_slot_size;
	Vec2 m_flag_init_position;
	int m_flagIndex = -1;
public:
	WeatherSlot();
	void placeWeatherCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer);
	int checkCardSpace(Player* currentPlayer) const;
	const std::vector<std::vector<std::shared_ptr<SpecialCard>>>& getWeatherCards() const { return m_weather_cards; };
	std::shared_ptr<SpecialCard> getCard(int playerIndex, int slotIndex) const;
	void setCardSlotSize(Vec2 card_slot_size) { m_card_slot_size = card_slot_size; };
	Vec2 getCardSlotSize() const { return m_card_slot_size; };
	void slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots = true);
	RectF getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);

	std::shared_ptr<SpecialCard>* getCardData(int playerIndex);

	void setFlagInitPosition(Vec2 flag_init_position) { m_flag_init_position = flag_init_position; }
	Vec2 getFlagInitPosition() const { return m_flag_init_position; }
	void setFlagIndex(int index) { m_flagIndex = index; }
	int getFlagIndex() const { return m_flagIndex; }

	bool hasFogCard() const;
	bool hasMudCard() const;
};

#endif
