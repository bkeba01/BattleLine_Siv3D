#include "core/Player.h"
#include "core/HoverManager.h"
#include "core/DragManager.h"
#include "core/GameState.h"
#include "core/Common.h"
#include "core/Flag.h"
#include "core/Slot.h"
#include "core/SpecialCard.h"
#include "core/WeatherSlot.h"
#include "core/ConspiracySlot.h"

Player::Player(int playerId, Deck &deck, Vec2 card_hand_size, Vec2 card_hand_space)
	: m_id(playerId)
	, m_card_hand_size(card_hand_size)
	, m_card_hand_space(card_hand_space)
{
    m_hand.reserve(ste_HandCardMakeNum);
    for(int i=static_cast<int>(ste_HandCardMinNum);i<=static_cast<int>(ste_HandCardMaxNum);i++)
    {
		if (auto cardOpt = deck.drawCard())
		{
			Card& card = *cardOpt;
			card.setCardHandSize(card_hand_size);
			card.setCardHandSpace(card_hand_space);
			m_hand.push_back(std::make_shared<Card>(card));
		}
    }
    m_hand_empty=false;
}

int Player::getId() const { return m_id; }

void Player::drawCard(Deck* deck)
{
    if (m_hand.size() > static_cast<size_t>(ste_HandCardMaxNum)) { // ste_HandCardMaxNum is 6, so this prevents drawing if hand is 7 or more.
        return;
    }

    if (auto cardOpt = deck->drawCard())
    {
        Card& card = *cardOpt;
        card.setCardHandSize(m_card_hand_size);
        card.setCardHandSpace(m_card_hand_space);
        m_hand.push_back(std::make_shared<Card>(card));
    }
    // If cardOpt is nullopt (deck is empty), do nothing.
}

const std::vector<std::shared_ptr<CardBase>>& Player::getHand() const { return m_hand; }

void Player::drawSpecialCard(SpecialDeck* deck)
{
	if (m_hand.size() > static_cast<size_t>(ste_HandCardMaxNum)) {
		return;
	}

	if (auto cardOpt = deck->drawCard())
	{
		SpecialCard& card = *cardOpt;
		card.setCardHandSize(m_card_hand_size);
		card.setCardHandSpace(m_card_hand_space);
		m_hand.push_back(std::make_shared<SpecialCard>(card));
	}
}

void Player::setChoiceCardIndex(int card_index) 
{
    if (card_index < static_cast<int>(ste_HandCardMinNum) || card_index >= static_cast<int>(m_hand.size())) 
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
    if (index < static_cast<int>(ste_HandCardMinNum) || index >= static_cast<int>(m_hand.size()))
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

std::shared_ptr<CardBase> Player::removeCardFromHandByIndex(int index)
{
    if (index < static_cast<int>(ste_HandCardMinNum) || index >= static_cast<int>(m_hand.size()))
    {
        throw std::out_of_range("Invalid card index");
    }
    auto card = m_hand[index];
    m_hand.erase(m_hand.begin() + index);
    if (m_hand.empty())
    {
        setHandIsEmpty(true);
    }
    return card;
}

void Player::addCardToHand(std::shared_ptr<CardBase> card, bool force)
{
    // 手札の上限チェック（7枚まで）
    // force=trueの場合は上限を無視（ReconCard用）
    if (!force && m_hand.size() >= static_cast<size_t>(ste_HandCardMakeNum))
    {
        return; // 手札が満杯なので追加しない
    }
    m_hand.push_back(card);
    setHandIsEmpty(false);
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
		card->setCardHandSpace(space);
	}
}

void Player::updateDrag(Array<RectF>& cardRects)
{
	m_dragManager.updateDrag(cardRects);
}


void Player::update()
{
	m_cardRects.clear();
	for (int i = static_cast<int>(ste_HandCardMinNum); i < m_hand.size(); ++i)
	{
		const double centerX = m_hand[i]->getCardHandSpace().x + m_hand[i]->getCardHandSize().x / 2 * i + (m_hand[i]->getCardHandSize().x / 2.0);
		m_cardRects << RectF{ Arg::center(centerX, m_hand[i]->getCardHandSpace().y), m_hand[i]->getCardHandSize().x, m_hand[i]->getCardHandSize().y };
	}

	// ドラッグ中のカードタイプを判定
	if (const auto heldIndex = m_dragManager.heldIndex())
	{
		int index = *heldIndex;
		if (index >= 0 && index < m_hand.size())
		{
			// 通常のCardか判定
			if (auto cardPtr = std::dynamic_pointer_cast<Card>(m_hand[index]))
			{
				m_draggedCardType = DraggedCardType::NormalCard;
			}
			// SpecialCardか判定
			else if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[index]))
			{
				SpecialCardCategory category = specialCardPtr->getCategory();
				if (category == ste_TroopCard)
				{
					m_draggedCardType = DraggedCardType::TroopCard;
				}
				else if (category == ste_WeatherTacticCard)
				{
					m_draggedCardType = DraggedCardType::WeatherCard;
				}
				else if (category == ste_ConspiracyTacticCard)
				{
					m_draggedCardType = DraggedCardType::ConspiracyCard;
				}
			}
		}
	}
	else
	{
		// ドラッグしていない場合はNone
		m_draggedCardType = DraggedCardType::None;
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
			const Vec2 droppedCardCenter = m_hand[droppedHandIndex]->getRect().center();

			bool droppedInSlot = false;

			// 通常のカードの場合
			if (auto cardPtr = std::dynamic_pointer_cast<Card>(m_hand[droppedHandIndex]))
			{
				for (auto& flag : gameState.getFlags())
				{
					Slot& currentSlot = gameState.getSlot(flag.getSlotIndex());
					int emptySlotIndex = currentSlot.checkCardSpace(gameState.getCurrentPlayer(), gameState);
					if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
					{
						const RectF slotRect = currentSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
						if (slotRect.contains(droppedCardCenter))
						{
							currentSlot.placeCard(gameState, *cardPtr, gameState.getCurrentPlayer());
							removeCardFromHand(droppedHandIndex);
							gameState.setWaitingForDeckChoice(true);
							m_dragManager.clearHover();
							droppedInSlot = true;
							break;
						}
					}
				}
			}
			// 特殊カードの場合
			else if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[droppedHandIndex]))
			{
				// スペシャルカード使用制限チェック
				if (!gameState.getCurrentPlayer()->getCanUseSpecialCard())
				{
					// 使用不可の場合は処理をスキップ
					return;
				}

				SpecialCardCategory category = specialCardPtr->getCategory();

				// 部隊カード - スロットに配置（通常カードと同じ）
				if (category == ste_TroopCard)
				{
					for (auto& flag : gameState.getFlags())
					{
						Slot& currentSlot = gameState.getSlot(flag.getSlotIndex());
						int emptySlotIndex = currentSlot.checkCardSpace(gameState.getCurrentPlayer(), gameState);
						if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
						{
							const RectF slotRect = currentSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
							if (slotRect.contains(droppedCardCenter))
							{
								currentSlot.placeSpecialCard(gameState, *specialCardPtr, gameState.getCurrentPlayer());
								removeCardFromHand(droppedHandIndex);
								gameState.setWaitingForDeckChoice(true);
								m_dragManager.clearHover();
								droppedInSlot = true;
								break;
							}
						}
					}
				}
				// 気象戦術カード - フラグの下に配置
				else if (category == ste_WeatherTacticCard)
				{
					for (auto& flag : gameState.getFlags())
					{
						WeatherSlot& currentWeatherSlot = gameState.getWeatherSlot(flag.getSlotIndex());
						int emptySlotIndex = currentWeatherSlot.checkCardSpace(gameState.getCurrentPlayer());
						if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
						{
							const RectF slotRect = currentWeatherSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
							if (slotRect.contains(droppedCardCenter))
							{
								currentWeatherSlot.placeWeatherCard(gameState, *specialCardPtr, gameState.getCurrentPlayer());
								removeCardFromHand(droppedHandIndex);
								gameState.setWaitingForDeckChoice(true);
								m_dragManager.clearHover();
								droppedInSlot = true;
								break;
							}
						}
					}
				}
				// 謀略戦術カード - 特殊デッキの近くに配置
				else if (category == ste_ConspiracyTacticCard)
				{
					ConspiracySlot& conspiracySlot = gameState.getConspiracySlot();
					int emptySlotIndex = conspiracySlot.checkCardSpace(gameState.getCurrentPlayer());
					if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
					{
						const RectF slotRect = conspiracySlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
						if (slotRect.contains(droppedCardCenter))
						{
							conspiracySlot.placeConspiracyCard(gameState, *specialCardPtr, gameState.getCurrentPlayer());
							removeCardFromHand(droppedHandIndex);
							gameState.setWaitingForDeckChoice(true);
							m_dragManager.clearHover();
							droppedInSlot = true;
						}
					}
				}

				// 配置成功時、フラグを更新
				if (droppedInSlot)
				{
					gameState.getCurrentPlayer()->setCanUseSpecialCard(false);
					gameState.getOpponentPlayer()->setCanUseSpecialCard(true);
				}
			}
		}
	}
}

void Player::handleDeckChoice(GameState& gameState)
{
	if (!gameState.isWaitingForDeckChoice()) {
		return;
	}

	// 通常デッキのクリック判定
	if (gameState.getDeck()->getRect().leftClicked())
	{
		gameState.drawFromNormalDeck();
		gameState.changePlayer();
	}
	// 特殊デッキのクリック判定
	else if (gameState.getSpecialDeck()->getRect().leftClicked())
	{
		gameState.drawFromSpecialDeck();
		gameState.changePlayer();
	}
}

void Player::draw(GameState& gameState)
{
	drawPlayerCards();
	drawHoveredAndHeldCards(gameState);
}

void Player::drawBacks()
{
	for (int i = 0; i < m_hand.size(); ++i)
	{
		// Calculate rect for opponent's cards
		const double centerX = m_card_hand_space.x + m_card_hand_size.x / 2 * i + (m_card_hand_size.x / 2.0);
		const RectF cardRect{ Arg::center(centerX, m_card_hand_space.y), m_card_hand_size.x, m_card_hand_size.y };

		m_hand[i]->setRect(cardRect);
		m_hand[i]->drawBack();
	}
}

void Player::drawPlayerCards()
{
	const auto& hoveredCardIndex = m_dragManager.hoveredIndex();
	const auto& heldCardIndex = m_dragManager.heldIndex();

	for (int i = static_cast<int>(ste_HandCardMinNum); i < m_hand.size(); ++i)
	{
		bool isHeld = heldCardIndex && (*heldCardIndex == i);
		bool isHovered = hoveredCardIndex && (*hoveredCardIndex == i);

		if (not isHeld && not isHovered)
		{
			m_hand[i]->setRect(m_cardRects[i]);

			// スペシャルカードで使用不可の場合はグレーアウト
			auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[i]);
			if (specialCardPtr && !m_canUseSpecialCard)
			{
				ScopedColorMul2D colorMul(ColorF(1.0, 0.5));
				m_hand[i]->draw();
			}
			else
			{
				m_hand[i]->draw();
			}
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
		m_hand[i]->setRect(enlargedCard);

		// スペシャルカードで使用不可の場合はグレーアウト
		auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[i]);
		if (specialCardPtr && !m_canUseSpecialCard)
		{
			ScopedColorMul2D colorMul(ColorF(1.0, 0.5));
			m_hand[i]->draw();
		}
		else
		{
			m_hand[i]->draw();
		}
	}

	if (currentHeldRectsIndex)
	{
		Vec2 cardSize = m_hand[*currentHeldRectsIndex]->getCardHandSize();
		const RectF originalDraggingRect = m_dragManager.getDraggingRect(cardSize.x, cardSize.y);
		const Vec2 draggedCardCenter = originalDraggingRect.center();

		// 通常のカードの場合
		if (auto cardPtr = std::dynamic_pointer_cast<Card>(m_hand[*currentHeldRectsIndex]))
		{
			for (auto& flag : gameState.getFlags())
			{
				Slot& currentSlot = gameState.getSlot(flag.getSlotIndex());
				int emptySlotIndex = currentSlot.checkCardSpace(gameState.getCurrentPlayer(),gameState);
				if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
				{
					const RectF slotRect = currentSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
					if (slotRect.contains(draggedCardCenter))
					{
						cardSize = currentSlot.getCardSlotSize();
						break;
					}
				}
			}
		}
		// 特殊カードの場合
		else if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[*currentHeldRectsIndex]))
		{
			SpecialCardCategory category = specialCardPtr->getCategory();

			// 部隊カード - スロットサイズに合わせる
			if (category == ste_TroopCard)
			{
				for (auto& flag : gameState.getFlags())
				{
					Slot& currentSlot = gameState.getSlot(flag.getSlotIndex());
					int emptySlotIndex = currentSlot.checkCardSpace(gameState.getCurrentPlayer(), gameState);
					if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
					{
						const RectF slotRect = currentSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
						if (slotRect.contains(draggedCardCenter))
						{
							cardSize = currentSlot.getCardSlotSize();
							break;
						}
					}
				}
			}
			// 気象戦術カード - 気象エリアのサイズに合わせる
			else if (category == ste_WeatherTacticCard)
			{
				for (auto& flag : gameState.getFlags())
				{
					WeatherSlot& currentWeatherSlot = gameState.getWeatherSlot(flag.getSlotIndex());
					int emptySlotIndex = currentWeatherSlot.checkCardSpace(gameState.getCurrentPlayer());
					if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
					{
						const RectF slotRect = currentWeatherSlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
						if (slotRect.contains(draggedCardCenter))
						{
							cardSize = currentWeatherSlot.getCardSlotSize();
							break;
						}
					}
				}
			}
			// 謀略戦術カード - 謀略エリアのサイズに合わせる
			else if (category == ste_ConspiracyTacticCard)
			{
				ConspiracySlot& conspiracySlot = gameState.getConspiracySlot();
				int emptySlotIndex = conspiracySlot.checkCardSpace(gameState.getCurrentPlayer());
				if (emptySlotIndex != static_cast<int>(ste_SlotCard_NonSpace))
				{
					const RectF conspiracyRect = conspiracySlot.getCardSlotRect(gameState, gameState.getCurrentPlayer()->getId(), emptySlotIndex, gameState.getCurrentPlayer()->getId());
					if (conspiracyRect.contains(draggedCardCenter))
					{
						cardSize = Vec2{ conspiracyRect.w, conspiracyRect.h };
					}
				}
			}
		}

		m_hand[*currentHeldRectsIndex]->setRect(m_dragManager.getDraggingRect(cardSize.x, cardSize.y));

		// スペシャルカードで使用不可の場合はグレーアウト
		auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(m_hand[*currentHeldRectsIndex]);
		if (specialCardPtr && !m_canUseSpecialCard)
		{
			ScopedColorMul2D colorMul(ColorF(1.0, 0.5));
			m_hand[*currentHeldRectsIndex]->draw();
		}
		else
		{
			m_hand[*currentHeldRectsIndex]->draw();
		}
	}
}
