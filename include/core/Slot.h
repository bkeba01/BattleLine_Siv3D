#ifndef SLOT_H
#define SLOT_H
#include <Siv3D.hpp>
#include <vector>

class Player;
class Card;
class Flag;
class GameState;

class Slot
{
	private:
		std::vector<std::vector<Card>> m_cards;
		bool m_card_space[2]; // 各プレイヤーのカードスペースが空かどうかを示す配列
		Vec2 m_card_slot_size;
		Vec2 m_flag_init_position;
		int m_flagIndex = -1;
	public:
		Slot();
		void placeCard(GameState& gameState, const Card& card, Player* currentPlayer);
		Card getCard(int playerIndex, int slotIndex) const;
		int checkCardSpace(Player* currentPlayer);
		void setCardSpace(bool isEmpty, int playerIndex);
		bool getCardSpace(int playerIndex);
		void setCardSlotSize(Vec2 card_slot_size) { m_card_slot_size = card_slot_size; };
		Vec2 getCardSlotSize() { return m_card_slot_size; };
		void slotdraw(GameState& gameState, int currentPlayerId);
		RectF getCardSlotRect(GameState& gameState, int playerIndex, int slotIndex, int currentPlayerId);
		void setFlagInitPosition(Vec2 flag_init_position) {
			m_flag_init_position = flag_init_position;
		}
		Vec2 getFlagInitPosition() { return m_flag_init_position; };
		const std::vector<std::vector<Card>>& getCards() const { return m_cards; };
		Card* getCardData(int playerIndex);
		void setFlagIndex(int index) { m_flagIndex = index; };
		int getFlagIndex() const { return m_flagIndex; };
};

#endif
