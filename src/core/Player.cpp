#include "core/Player.h"
#include "core/HoverManager.h"
#include "core/DragManager.h"
#include "core/GameState.h"
#include "core/Common.h"

Player::Player(int playerId, Deck &deck, Vec2 card_hand_size, Vec2 card_hand_space) : m_id(playerId) 
{
    m_hand.reserve(ste_HandCardMakeNum);
    for(int i=ste_HandCardMinNum;i<=ste_HandCardMaxNum;i++)
    {
        try 
        {
            Card card = deck.drawCard();
			card.setCardHandSize(card_hand_size);
			card.setCardHandSpace(card_hand_space);
            m_hand.push_back(card);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Player " + std::to_string(m_id) + " cannot draw a card: " + e.what());
        }
    }
    m_hand_empty=false;

	for (int i = ste_HandCardMinNum; i < m_hand.size(); ++i)
	{
		const double centerX = m_hand[i].getCardHandSpace().x + m_hand[i].getCardHandSize().x / 2 * i + (m_hand[i].getCardHandSize().x / 2.0);
		m_cardRects << RectF{ Arg::center(centerX, m_hand[i].getCardHandSpace().y), m_hand[i].getCardHandSize().x, m_hand[i].getCardHandSize().y };
	}
}

int Player::getId() const { return m_id; }

int Player::drawCard(Deck* deck) 
{
    if (m_hand.size() >ste_HandCardMaxNum) {
        return ste_HandCardFull; // Hand is full, do not draw
    }
    try {
        Card card = deck->drawCard();
        //push_backは、ベクターの末尾に要素を追加する。
        if(card.getValue() == ste_NoneCard && card.getColor() == ste_NoneColor)
        {
            return ste_NoneDeck;
        }
        m_hand.push_back(card);
		const double centerX = m_hand.back().getCardHandSpace().x + m_hand.back().getCardHandSize().x / 2 * (m_hand.size() - 1) + (m_hand.back().getCardHandSize().x / 2.0);
		m_cardRects << RectF{ Arg::center(centerX, m_hand.back().getCardHandSpace().y), m_hand.back().getCardHandSize().x, m_hand.back().getCardHandSize().y };
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Player " + std::to_string(m_id) + " cannot draw a card: " + e.what());
    }
    return 0;
}

const std::vector<Card>& Player::getHand() const { return m_hand; }

void Player::setChoiceCardIndex(int card_index) 
{
    if (card_index < ste_HandCardMinNum || card_index >= static_cast<int>(m_hand.size())) 
    {
        throw std::out_of_range("Invalid card index");
    }
    m_card_index = card_index;
}

int Player::getChoiceCardIndex() const 
{
    return m_card_index;
}

int Player::removeCardFromHand(int index) 
{
    if (index < ste_HandCardMinNum || index >= static_cast<int>(m_hand.size())) 
    {
        throw std::out_of_range("Invalid card index");
    }
    m_hand.erase(m_hand.begin() + index);
	m_cardRects.remove_at(index);
    if(m_hand.empty())
    {
        setHandIsEmpty(true);
        return -1;
    }
    return 0;
}

bool Player::getHandIsEmpty() const 
{
    return m_hand_empty;
}

void Player::setHandIsEmpty(bool empty)
{
    m_hand_empty = empty;
}

void Player::updateDrag(Array<RectF>& cardRects)
{
	m_dragManager.updateDrag(cardRects);
}


void Player::drawPlayerCards()
{
	const auto& hoveredCardIndex = m_dragManager.hoveredIndex();
	const auto& heldCardIndex = m_dragManager.heldIndex();

	for (int i = ste_HandCardMinNum; i < m_hand.size(); ++i)
	{
		bool isHeld = heldCardIndex && (*heldCardIndex == i);
		bool isHovered = hoveredCardIndex && (*hoveredCardIndex == i);

		if (not isHeld && not isHovered)
		{
			m_hand[i].draw(m_cardRects[i]);
		}
	}
}

void Player::drawHoveredAndHeldCards(GameState& gameState)
{
	const auto& currentHoveredRectsIndex = m_dragManager.hoveredIndex();
	const auto& currentHeldRectsIndex = m_dragManager.heldIndex();

	if (currentHoveredRectsIndex && !currentHeldRectsIndex)
	{
		const int i = *currentHoveredRectsIndex;
		const RectF enlargedCard = m_cardRects[i].scaledAt(m_cardRects[i].center(), 1.15).moveBy(0, -20);
		m_hand[i].draw(enlargedCard);
	}

	if (currentHeldRectsIndex)
	{
		Vec2 cardSize = m_hand[*currentHeldRectsIndex].getCardHandSize();
		const RectF originalDraggingRect = m_dragManager.getDraggingRect(cardSize.x, cardSize.y);
		const Vec2 draggedCardCenter = originalDraggingRect.center();

		for (auto& flag : gameState.getFlags())
		{
			int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
			if (emptySlotIndex != ste_SlotCard_NonSpace)
			{
				const RectF slotRect = flag.getCardSlotRect(gameState.getCurrentPlayer()->getId(), emptySlotIndex);
				if (slotRect.contains(draggedCardCenter))
				{
					cardSize = flag.getCardSlotSize();
					break;
				}
			}
		}

		const RectF draggingRect = m_dragManager.getDraggingRect(cardSize.x, cardSize.y);
		m_hand[*currentHeldRectsIndex].draw(draggingRect);
	}
}

void Player::drawHand(GameState& gameState)
{
	if (m_cardRects.size() != m_hand.size())
	{
		m_cardRects.clear();
		for (int i = ste_HandCardMinNum; i < m_hand.size(); ++i)
		{
			const double centerX = m_hand[i].getCardHandSpace().x + m_hand[i].getCardHandSize().x / 2 * i + (m_hand[i].getCardHandSize().x / 2.0);
			m_cardRects << RectF{ Arg::center(centerX, m_hand[i].getCardHandSpace().y), m_hand[i].getCardHandSize().x, m_hand[i].getCardHandSize().y };
		}
	}

	m_dragManager.updateDrag(m_cardRects);

	if (MouseL.up())
	{
		if (const auto heldIndex = m_dragManager.heldIndex())
		{
			const int droppedHandIndex = *heldIndex;
			const RectF& droppedRect = m_dragManager.getDraggingRect(m_hand[droppedHandIndex].getCardHandSize().x, m_hand[droppedHandIndex].getCardHandSize().y);
			const Vec2 droppedCardCenter = droppedRect.center();

			for (auto& flag : gameState.getFlags())
			{
				int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
				if (emptySlotIndex != ste_SlotCard_NonSpace)
				{
					const RectF slotRect = flag.getCardSlotRect(gameState.getCurrentPlayer()->getId(), emptySlotIndex);
					if (slotRect.contains(droppedCardCenter))
					{
						flag.placeCard(m_hand[droppedHandIndex], gameState.getCurrentPlayer());
						removeCardFromHand(droppedHandIndex);
						break; 
					}
				}
			}
		}
	}

	drawPlayerCards();
	drawHoveredAndHeldCards(gameState);
}
