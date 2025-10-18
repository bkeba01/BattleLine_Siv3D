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

void Player::drawHand(GameState& gameState)
{
	const auto& hoveredCardIndex = m_dragManager.hoveredIndex();
	const auto& heldCardIndex = m_dragManager.heldIndex();
	Array<RectF> cardRects;
	for (int i = ste_HandCardMinNum; i < m_hand.size(); ++i)
	{
		// プレイヤーの手札にあるカードの Rect を作成
		const double centerX = m_hand[i].getCardHandSpace().x + m_hand[i].getCardHandSize().x / 2 * i + (m_hand[i].getCardHandSize().x / 2.0);
		cardRects << RectF{ Arg::center(centerX, m_hand[i].getCardHandSpace().y), m_hand[i].getCardHandSize().x, m_hand[i].getCardHandSize().y };
		const RectF cardRect{ Arg::center(centerX, m_hand[i].getCardHandSpace().y), m_hand[i].getCardHandSize().x, m_hand[i].getCardHandSize().y };
		bool isHeld = heldCardIndex && (*heldCardIndex == i);
		bool isHovered = hoveredCardIndex && (*hoveredCardIndex == i);
		if (isHeld or isHovered)
		{
			continue;
		}
		m_hand[i].draw(cardRect);
	}
	this->updateDrag(cardRects);

	// ホバーされているカードを最後に描画して、他のカードより手前に表示
	if (m_dragManager.isDragging())
	{
		Vec2 cardSize = m_hand[ste_Player1].getCardHandSize();
		bool slotFound = false;

		if (const auto heldIndex = m_dragManager.heldIndex())
		{
			const RectF originalDraggingRect = m_dragManager.getDraggingRect(cardSize.x, cardSize.y);
			const Vec2 draggedCardCenter = originalDraggingRect.center();

			for (auto& flag : gameState.getFlags())
			{
				int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
				if (emptySlotIndex != ste_SlotCard_NonSpace)
				{
					for (; emptySlotIndex <=ste_SlotCardMaxNum; ++emptySlotIndex)
					{
						const RectF slotRect = flag.getCardSlotRect(gameState.getCurrentPlayer()->getId(), emptySlotIndex);

						if (slotRect.contains(draggedCardCenter))
						{
							cardSize = flag.getCardSlotSize();
							slotFound = true;
							break;
						}
					}
				}
				if (slotFound)
				{
					break;
				}
			}
		}

		const RectF draggingRect = m_dragManager.getDraggingRect(cardSize.x, cardSize.y);
		if (m_dragManager.heldIndex() != none)
		{
			int holdIndex = m_dragManager.heldIndex().value();
			m_hand[holdIndex].draw(draggingRect);
		}
	}
	else if (MouseL.up() && m_dragManager.getDraggingLastFrame()) // ドラッグが終了したフレームの処理
	{
		if (const auto heldIndex = m_dragManager.getheldIndexLastFrame()) // 👈 heldIndex を保持しておく
		{
			const RectF droppedRect = m_dragManager.getDraggingRect(
				m_hand[*heldIndex].getCardHandSize().x,
				m_hand[*heldIndex].getCardHandSize().y);
			const Vec2 droppedCardCenter = droppedRect.center();

			for (auto& flag : gameState.getFlags())
			{
				int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
				if (emptySlotIndex != ste_SlotCard_NonSpace)
				{
					const RectF slotRect = flag.getCardSlotRect(
						gameState.getCurrentPlayer()->getId(), emptySlotIndex);

					if (slotRect.contains(droppedCardCenter))
					{
						flag.placeCard(m_hand[*heldIndex], gameState.getCurrentPlayer());
						removeCardFromHand(*heldIndex);
						break;
					}
				}
			}
		}
		/*
		const int index = *heldCardIndex;
		const RectF droppedRect = m_dragManager.getDraggingRect(m_hand[index].getCardHandSize().x, m_hand[index].getCardHandSize().y);
		const Vec2 droppedCardCenter = droppedRect.center();
		bool placed = false;

		for (auto& flag : gameState.getFlags())
		{
			int emptySlotIndex = flag.checkCardSpace(gameState.getCurrentPlayer());
			if (emptySlotIndex != ste_SlotCard_NonSpace)
			{
				const RectF slotRect{ flag.getPos().x - (flag.getCardSlotSize().x / 2), flag.getTexture().height() / 2 + flag.getPos().y + 20 + emptySlotIndex * (flag.getCardSlotSize().y / 3), flag.getCardSlotSize().x, flag.getCardSlotSize().y };

				if (slotRect.contains(droppedCardCenter))
				{
					// カードを配置
					flag.placeCard(m_hand[index], gameState.getCurrentPlayer());
					// 手札から削除
					removeCardFromHand(index);
					placed = true;
					break;
				}
			}
		}
		*/
	}
	else if (hoveredCardIndex)
	{
		const int i = *hoveredCardIndex;
		const double centerX = m_hand[ste_Player1].getCardHandSpace().x + m_hand[ste_Player1].getCardHandSize().x / 2 * i + (m_hand[ste_Player1].getCardHandSize().x / 2.0);
		const RectF cardRect{ Arg::center(centerX, m_hand[ste_Player1].getCardHandSpace().y), m_hand[ste_Player1].getCardHandSize().x, m_hand[ste_Player1].getCardHandSize().y };
		const RectF enlargedCard = cardRect.scaledAt(cardRect.center(), 1.15).moveBy(0, -20);
		m_hand[i].draw(enlargedCard);
	}
}
