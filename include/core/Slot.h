#ifndef SLOT_H
#define SLOT_H
#include <Siv3D.hpp>
#include <vector>
#include <memory>

class Player;
class Card;
class CardBase;
class Flag;
class GameState;
class SpecialCard;

class Slot
{
	private:
		std::vector<std::vector<std::shared_ptr<CardBase>>> m_cards;
		bool m_card_space[2]; // 各プレイヤーのカードスペースが空かどうかを示す配列
		Vec2 m_card_slot_size;
		Vec2 m_flag_init_position;
		int m_flagIndex = -1;
	public:
		Slot();
		void placeCard(GameState& gameState, const Card& card, Player* currentPlayer);
		void placeSpecialCard(GameState& gameState, const SpecialCard& card, Player* currentPlayer);
		std::shared_ptr<CardBase> getCard(int playerIndex, int slotIndex) const;
		int checkCardSpace(Player* currentPlayer, GameState& gameState);
		void setCardSpace(bool isEmpty, int playerIndex);
		bool getCardSpace(int playerIndex);
		void setCardSlotSize(Vec2 card_slot_size) { m_card_slot_size = card_slot_size; };
		Vec2 getCardSlotSize() { return m_card_slot_size; };
		void slotdraw(GameState& gameState, int currentPlayerId, bool showEmptySlots = true);
		RectF getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);
		void setFlagInitPosition(Vec2 flag_init_position) {
			m_flag_init_position = flag_init_position;
		}
		Vec2 getFlagInitPosition() { return m_flag_init_position; };
		const std::vector<std::vector<std::shared_ptr<CardBase>>>& getCards() const { return m_cards; };
		std::vector<std::vector<std::shared_ptr<CardBase>>>& getCards() { return m_cards; };
		std::shared_ptr<CardBase>* getCardData(int playerIndex);
		void setFlagIndex(int index) { m_flagIndex = index; };
		int getFlagIndex() const { return m_flagIndex; };
};

#endif
