#ifndef PLAYER_H
#define PLAYER_H

#include <stdexcept>
#include <string>
#include <vector>
#include "Card.h"
#include "Deck.h"
#include "DragManager.h"
class GameState;

class Player 
{
    private:
        int m_id;
        std::vector<Card> m_hand;
        int m_card_index; 
        bool m_hand_empty; // 手札が空かどうかを保存する変数
        DragManager m_dragManager;
		Array<RectF> m_cardRects;

    public:
        Player(int playerId,Deck &deck,Vec2,Vec2);
        
        int getId() const;
        int drawCard(Deck* deck);
        const std::vector<Card>& getHand() const;
        void setChoiceCardIndex(int card_index);

        int getChoiceCardIndex() const;

        int removeCardFromHand(int index);
        void setHandIsEmpty(bool empty);
        bool getHandIsEmpty() const;

        void updateDrag(Array<RectF>& cardRects);
        const DragManager& getDragManager() const { return m_dragManager; }

		void drawHand(GameState& gameState);

	private:
		void drawPlayerCards();

		void drawHoveredAndHeldCards(GameState& gameState);
        
};
#endif
