#ifndef CONSPIRACYSLOT_H
#define CONSPIRACYSLOT_H

#include <Siv3D.hpp>
#include <vector>
#include <memory>
#include "core/Common.h"

class Player;
class GameState;
class SpecialCard;

class ConspiracySlot
{
private:
	std::vector<std::vector<std::shared_ptr<SpecialCard>>> m_conspiracy_cards;
	Vec2 m_card_slot_size;
	Vec2 m_special_deck_position;
public:
	ConspiracySlot();
	void placeConspiracyCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer);
	int checkCardSpace(Player* currentPlayer) const;
	const std::vector<std::vector<std::shared_ptr<SpecialCard>>>& getConspiracyCards() const { return m_conspiracy_cards; };
	std::shared_ptr<SpecialCard> getCard(int playerIndex, int slotIndex) const;
	void setCardSlotSize(Vec2 card_slot_size) { m_card_slot_size = card_slot_size; };
	Vec2 getCardSlotSize() const { return m_card_slot_size; };
	void slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots = true);
	RectF getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);

	std::shared_ptr<SpecialCard>* getCardData(int playerIndex);

	void setSpecialDeckPosition(Vec2 position) { m_special_deck_position = position; }
	Vec2 getSpecialDeckPosition() const { return m_special_deck_position; }
};

#endif
