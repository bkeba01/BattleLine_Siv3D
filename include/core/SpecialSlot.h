#ifndef SPECIALSLOT_H
#define SPECIALSLOT_H

#include <Siv3D.hpp>
#include <vector>
#include <memory>
#include "core/Common.h"

class Player;
class Card;
class Flag;
class GameState;
class SpecialCard;

class SpecialSlot
{
private:
	std::vector<std::vector<std::shared_ptr<SpecialCard>>> m_flag_lower_cards;
	std::vector<std::vector<std::shared_ptr<SpecialCard>>> m_specialdeck_lower_cards;
	Vec2 m_specialcard_slot_size;
	Vec2 m_flag_init_position;
	int m_flagIndex = -1;
public:
	SpecialSlot();
	void placeFlagLowerSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer);
	void placeSpecialDeckLowerSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer);
	int checkCardSpace(Player* currentPlayer, const SpecialCard& card);
	const std::vector<std::vector<std::shared_ptr<SpecialCard>>>& getFlagLowerCards() const { return m_flag_lower_cards; };
	const std::vector<std::vector<std::shared_ptr<SpecialCard>>>& getSpecialDeckLowerCards() const { return m_specialdeck_lower_cards; };
	std::shared_ptr <SpecialCard> getCard(int playerIndex, int slotIndex) const;
	void setCardSlotSize(Vec2 card_slot_size) { m_specialcard_slot_size = card_slot_size; };
	Vec2 getCardSlotSize() { return m_specialcard_slot_size; };
	void slotdraw(GameState& gameState, int currentPlayerId);
	RectF getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);
	RectF getFlagLowerSpecialCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);
	RectF getSpecialDeckLowerSpecialCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);

	std::shared_ptr <SpecialCard>* getCardData(int playerIndex);

	void setFlagInitPosition(Vec2 flag_init_position) { m_flag_init_position = flag_init_position; }
	Vec2 getFlagInitPosition() { return m_flag_init_position; }
	void setFlagIndex(int index) { m_flagIndex = index; }
	int getFlagIndex() const { return m_flagIndex; }
};

#endif
