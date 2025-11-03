#ifndef PLAYER_H
#define PLAYER_H

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include "Card.h"
#include "Deck.h"
#include "SpecialDeck.h"
#include "SpecialCard.h"
#include "DragManager.h"
#include "Common.h"
#include <optional>
class GameState;
class CardBase;
class DragManager;

class Player
{
    private:
        int m_id;
        std::vector<std::shared_ptr<CardBase>> m_hand;
        int m_card_index;
        bool m_hand_empty; // 手札が空かどうかを保存する変数
        DragManager m_dragManager;
		Array<RectF> m_cardRects;
		Array<RectF> m_opponentCardRects;
		Vec2 m_card_hand_size;
		Vec2 m_card_hand_space;
		DraggedCardType m_draggedCardType = DraggedCardType::None;
		bool m_canUseSpecialCard = true;

    public:
        Player(int playerId,Deck &deck,Vec2,Vec2);

        int getId() const;
        void drawCard(Deck* deck);
        void drawSpecialCard(SpecialDeck* deck);
        const std::vector<std::shared_ptr<CardBase>>& getHand() const;
        void setChoiceCardIndex(int card_index);

        int getChoiceCardIndex() const;

        int removeCardFromHand(int index);
        std::shared_ptr<CardBase> removeCardFromHandByIndex(int index);
        void addCardToHand(std::shared_ptr<CardBase> card, bool force = false);
        void setHandIsEmpty(bool empty);
        bool getHandIsEmpty() const;

		void setHandSpace(Vec2 space);

        void updateDrag(Array<RectF>& cardRects);
        const DragManager& getDragManager() const { return m_dragManager; }

		DraggedCardType getDraggedCardType() const { return m_draggedCardType; }
		void setDraggedCardType(DraggedCardType type) { m_draggedCardType = type; }

		void update();
		void handleInput(GameState& gameState);
		void handleDeckChoice(GameState& gameState);
		void draw(GameState& gameState);
		void drawBacks();

		//void drawopponentBacks() const;

		void setCanUseSpecialCard(bool canUse) { m_canUseSpecialCard = canUse; }
		bool getCanUseSpecialCard() const { return m_canUseSpecialCard; }

	private:
		void drawPlayerCards();

		void drawHoveredAndHeldCards(GameState& gameState);
        
};
#endif
