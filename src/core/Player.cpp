#include "core/Player.h"
#include "core/HoverManager.h"
#include "core/DragManager.h"
#include "core/GameState.h"
#include "core/Common.h"

Player::Player(int playerId, Deck &deck, Vec2 card_hand_size, Vec2 card_hand_space) 
	: m_id(playerId) 
	, m_card_hand_size(card_hand_size)
	, m_card_hand_space(card_hand_space)
{
    m_hand.reserve(ste_HandCardMakeNum);
    for(int i=ste_HandCardMinNum;i<=ste_HandCardMaxNum;i++)
    {
		if (auto cardOpt = deck.drawCard())
		{
			Card& card = *cardOpt;
			card.setCardHandSize(card_hand_size);
			card.setCardHandSpace(card_hand_space);
			m_hand.push_back(card);
		}
    }
    m_hand_empty=false;
}

int Player::getId() const { return m_id; }

void Player::drawCard(Deck* deck) 
{
    if (m_hand.size() > ste_HandCardMaxNum) { // ste_HandCardMaxNum is 6, so this prevents drawing if hand is 7 or more.
        return;
    }

    if (auto cardOpt = deck->drawCard())
    {
        Card& card = *cardOpt;
        card.setCardHandSize(m_card_hand_size);
        card.setCardHandSpace(m_card_hand_space);
        m_hand.push_back(card);
    }
    // If cardOpt is nullopt (deck is empty), do nothing.
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

void Player::setHandSpace(Vec2 space)
{
	m_card_hand_space = space;
	for (auto& card : m_hand)
	{
		card.setCardHandSpace(space);
	}
}

void Player::updateDrag(Array<RectF>& cardRects)
{
	m_dragManager.updateDrag(cardRects);
}


void Player::update()
{
	m_cardRects.clear();
	for (int i = ste_HandCardMinNum; i < m_hand.size(); ++i)
	{
		const double centerX = m_hand[i].getCardHandSpace().x + m_hand[i].getCardHandSize().x / 2 * i + (m_hand[i].getCardHandSize().x / 2.0);
		m_cardRects << RectF{ Arg::center(centerX, m_hand[i].getCardHandSpace().y), m_hand[i].getCardHandSize().x, m_hand[i].getCardHandSize().y };
	}
}

void Player::handleInput(GameState& gameState)
{
	m_dragManager.updateDrag(m_cardRects);

	if (m_dragManager.wasJustDropped())
	{
		if (const auto droppedIndex = m_dragManager.droppedIndex())
		{
			const int droppedHandIndex = *droppedIndex;
			const Vec2 droppedCardCenter = m_hand[droppedHandIndex].getRect().center();

			bool droppedInSlot = false;
			for (auto& flag : gameState.getFlags())
			{
				int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
				if (emptySlotIndex != ste_SlotCard_NonSpace)
				{
const RectF slotRect = flag.getCardSlotRect(gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
					if (slotRect.contains(droppedCardCenter))
					{
						flag.placeCard(m_hand[droppedHandIndex], gameState.getCurrentPlayer());
						removeCardFromHand(droppedHandIndex);
						gameState.changePlayer();
						m_dragManager.clearHover();
						droppedInSlot = true;
						break;
					}
				}
			}
		}
	}
}

void Player::draw(GameState& gameState)
{
	drawPlayerCards();
	drawHoveredAndHeldCards(gameState);
}

void Player::drawBacks() const
{
	for (int i = 0; i < m_hand.size(); ++i)
	{
		// Create a mutable copy to set the rect
		Card card = m_hand[i];
		card.setRect(m_cardRects[i]);
		card.drawBack();
	}
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
			m_hand[i].setRect(m_cardRects[i]);
			m_hand[i].draw();
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
		m_hand[i].setRect(enlargedCard);
		m_hand[i].draw();
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
				const RectF slotRect = flag.getCardSlotRect(gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
				if (slotRect.contains(draggedCardCenter))
				{
					cardSize = flag.getCardSlotSize();
					break;
				}
			}
		}

		m_hand[*currentHeldRectsIndex].setRect(m_dragManager.getDraggingRect(cardSize.x, cardSize.y));
		m_hand[*currentHeldRectsIndex].draw();
	}
}
