#include "core/GameState.h"
#include "core/Common.h"
#include "core/Flag.h"
#include "core/Slot.h"
#include "core/WeatherSlot.h"
#include "core/ConspiracySlot.h"
#include "core/SpecialCard.h"
#include <algorithm>
int checkSerialValue(int playerindex,int flagindex,std::vector<Flag>& m_flags,int serial_count=0);

GameState::GameState(Player player1, Player player2, Deck deck, SpecialDeck specialDeck)
    : m_player1(player1), m_player2(player2), m_deck(deck), m_special_deck(specialDeck)
{
    m_flags.reserve(ste_FlagMakeNum);
	m_slots.reserve(ste_FlagMakeNum);
	m_weatherSlots.reserve(ste_FlagMakeNum);

    for(int i=ste_FlagMinNum;i<=ste_FlagMaxNum;++i)
    {
        m_flags.emplace_back(i);
		m_slots.emplace_back();
		m_weatherSlots.emplace_back();
		m_flags[i].setSlotIndex(i);
		m_slots[i].setFlagIndex(i);
		m_weatherSlots[i].setFlagIndex(i);
    }

}

Player* GameState::getPlayer1() { return &m_player1; }

Player* GameState::getPlayer2() { return &m_player2; }

Deck* GameState::getDeck() { return &m_deck; }

SpecialDeck* GameState::getSpecialDeck() { return &m_special_deck; }

std::vector<Flag>& GameState::getFlags() { return m_flags; }

Slot& GameState::getSlot(int flagIndex) {
    return m_slots[flagIndex];
}

WeatherSlot& GameState::getWeatherSlot(int flagIndex) {
    return m_weatherSlots[flagIndex];
}

ConspiracySlot& GameState::getConspiracySlot() {
    return m_conspiracySlot;
}

Flag& GameState::getFlag(int slotIndex) {
    return m_flags[slotIndex];
}

void GameState::setCurrentPlayer(Player* player)
{
	m_currentPlayer = player;
	m_opponentPlayer = (m_currentPlayer->getId() == ste_Player1) ? getPlayer2() : getPlayer1();
}

Player* GameState::getCurrentPlayer() const { return m_currentPlayer; }

Player* GameState::getOpponentPlayer() const { return m_opponentPlayer; }

bool GameState::getFinished() { return finished; }

void GameState::autoSetFinished()
{
    for(int i=ste_PlayerMin;i<=ste_PlayerMax;++i)
    {
        m_player_score[i]= ste_Flag_NonStatus;
    }

    for(int playerindex=ste_PlayerMin;playerindex<=ste_PlayerMax;++playerindex)
    {
        for(int flagindex=ste_FlagMinNum;flagindex<=ste_FlagMaxNum;++flagindex)
        {
            if(m_flags[flagindex].getFlagStatus()==playerindex)
            {
                m_player_score[playerindex]++;
            }
        }
    }
    std::cout<<"Player 0 score:"<<m_player_score[PLAYER1]<<", Player 1 score:"<<m_player_score[PLAYER2]<<std::endl;
    if(m_player_score[PLAYER1]>=ste_FlagWinThreshold || m_player_score[PLAYER2]>= ste_FlagWinThreshold)
    {
        finished=true;
        m_winner=(m_player_score[PLAYER1]>m_player_score[PLAYER2])?PLAYER1:PLAYER2;
    }
    else if(m_player_score[PLAYER1]>=ste_FlagWinSerialThreshold || m_player_score[PLAYER2]>= ste_FlagWinSerialThreshold)
    {
        m_winner=checkSerialValue(0,0,m_flags);
        if(m_winner!=NONE)
        {
            finished=true;
        }
        else
        {
            finished=false;
            m_winner=NONE;
        }
    }
    else
    {
        finished=false;
        m_winner=NONE;
    }
}

int checkSerialValue(int playerIndex, int flagIndex, std::vector<Flag>& m_flags, int serialCount)
{
    if (flagIndex >= static_cast<int>(m_flags.size()))
        return NONE;

    if (m_flags[flagIndex].getFlagStatus() == playerIndex)
    {
        serialCount++;

        if (serialCount >= ste_FlagWinSerialThreshold)
            return playerIndex;

        // 次のフラグへ
        return checkSerialValue(playerIndex, flagIndex + 1, m_flags, serialCount);
    }
    else
    {
        // 連続が途切れたのでリセットして次へ
        return checkSerialValue(playerIndex, flagIndex + 1, m_flags);
    }
}

void GameState::setWinner(int winner)
{
    m_winner = winner;
}
int GameState::getWinner()
{
    return m_winner;
}
void GameState::changePlayer()
{
	// カードを引く処理は削除（山札選択後に個別に引く）
	m_currentPlayer = (m_currentPlayer->getId() == ste_Player1) ? getPlayer2() : getPlayer1();
	m_opponentPlayer = (m_currentPlayer->getId() == ste_Player1) ? getPlayer2() : getPlayer1();
}

bool GameState::isWaitingForDeckChoice() const
{
	return m_waiting_for_deck_choice;
}

void GameState::setWaitingForDeckChoice(bool waiting)
{
	m_waiting_for_deck_choice = waiting;
}

void GameState::drawFromNormalDeck()
{
	m_currentPlayer->drawCard(&m_deck);
	m_waiting_for_deck_choice = false;
}

void GameState::drawFromSpecialDeck()
{
	m_currentPlayer->drawSpecialCard(&m_special_deck);
	m_waiting_for_deck_choice = false;
}

void GameState::startReconCard()
{
	m_recon_mode = true;
	m_recon_phase = 0; // 山札選択フェーズ
	m_recon_drawn_cards.clear();
	m_recon_card_from_special.clear();
	m_recon_selected_hand_indices.clear();
	m_recon_selected_deck_indices.clear();
	m_recon_viewing_deck = false;
	m_recon_viewing_special = false;
	m_recon_scroll_offset = 0.0; // スクロールオフセットをリセット
	m_recon_cards_to_return.clear();
	m_recon_return_to_special.clear();
}

void GameState::startDeploymentCard()
{
	m_deployment_mode = true;
	m_deployment_source_flag = -1;
	m_deployment_source_slot = -1;
}

void GameState::startEscapeCard()
{
	m_escape_mode = true;
	m_escape_target_flag = -1;
	m_escape_target_slot = -1;
}

void GameState::startBetrayalCard()
{
	m_betrayal_mode = true;
	m_betrayal_source_flag = -1;
	m_betrayal_source_slot = -1;
}

// ========================================
// ネットワークマルチプレイ用メソッドの実装
// ========================================

#include "../Multiplayer_Photon.hpp"

void GameState::setNetwork(s3d::Multiplayer_Photon* network)
{
	m_network = network;
}

bool GameState::isMultiplayer() const
{
	return m_is_multiplayer;
}

bool GameState::isHost() const
{
	return m_is_host;
}

bool GameState::isMyTurn() const
{
	if (!m_is_multiplayer)
		return true; // ローカルモードでは常にtrue

	// マルチプレイモード：自分のプレイヤーのターンかチェック
	int currentPlayerIndex = (m_currentPlayer == &m_player1) ? 0 : 1;
	return (currentPlayerIndex == m_local_player_index);
}

int GameState::getLocalPlayerIndex() const
{
	return m_local_player_index;
}

void GameState::setMultiplayerMode(bool isMultiplayer, bool isHost)
{
	m_is_multiplayer = isMultiplayer;
	m_is_host = isHost;
	m_local_player_index = isHost ? 0 : 1; // ホストがPlayer1、クライアントがPlayer2
}

void GameState::setGameSeed(uint32_t seed)
{
	m_game_seed = seed;
}

uint32_t GameState::getGameSeed() const
{
	return m_game_seed;
}

// ========================================
// ネットワークイベント送信メソッド
// ========================================

void GameState::sendCardPlacedEvent(int flagIndex, int slotIndex, int cardId, bool isSpecialCard, int specialCardType)
{
	if (!m_network || !m_is_multiplayer)
		return;

	int color = cardId / 100;
	int value = cardId % 100;

	std::cout << "[Network] <<< Sending CARD_PLACED: flag=" << flagIndex << ", slot=" << slotIndex << ", cardId=" << cardId << " (color=" << color << ", value=" << value << "), isSpecial=" << isSpecialCard << ", type=" << specialCardType << std::endl;

	s3d::Array<int32> data = { flagIndex, slotIndex, cardId, isSpecialCard ? 1 : 0, specialCardType };
	m_network->sendEvent(EVENT_CARD_PLACED, data);

	std::cout << "[Network] Sent CARD_PLACED successfully" << std::endl;
}

void GameState::sendDeckChoiceEvent(int deckType)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending DECK_CHOICE: type=" << deckType << std::endl;

	m_network->sendEvent(EVENT_DECK_CHOICE, static_cast<int32>(deckType));

	std::cout << "[Network] Sent DECK_CHOICE successfully" << std::endl;
}

void GameState::sendGameInitEvent(uint32_t seed)
{
	if (!m_network || !m_is_multiplayer)
		return;

	s3d::Array<int32> data = { static_cast<int32>(seed), m_local_player_index };
	m_network->sendEvent(EVENT_GAME_INIT, data);

	std::cout << "[Network] Sent GAME_INIT: seed=" << seed << std::endl;
}

void GameState::sendPlayerReadyEvent()
{
	if (!m_network || !m_is_multiplayer)
		return;

	m_network->sendEvent(EVENT_PLAYER_READY, static_cast<int32>(1));

	std::cout << "[Network] Sent PLAYER_READY" << std::endl;
}

void GameState::sendDeckSyncEvent(const s3d::Array<int32>& deckOrder)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending DECK_SYNC: " << deckOrder.size() << " cards" << std::endl;
	m_network->sendEvent(EVENT_DECK_SYNC, deckOrder);
	std::cout << "[Network] Sent DECK_SYNC successfully" << std::endl;
}

void GameState::sendSpecialDeckSyncEvent(const s3d::Array<int32>& deckOrder)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending SPECIAL_DECK_SYNC: " << deckOrder.size() << " cards" << std::endl;
	m_network->sendEvent(EVENT_SPECIAL_DECK_SYNC, deckOrder);
	std::cout << "[Network] Sent SPECIAL_DECK_SYNC successfully" << std::endl;
}

// ========================================
// ネットワークイベント受信処理メソッド
// ========================================

void GameState::onCardPlacedReceived(int flagIndex, int slotIndex, int cardId, bool isSpecialCard, int specialCardType)
{
	std::cout << "[Network] >>> Received CARD_PLACED: flag=" << flagIndex << ", slot=" << slotIndex << ", card=" << cardId << ", isSpecial=" << isSpecialCard << ", type=" << specialCardType << std::endl;
	std::cout << "[Network] Local player index: " << m_local_player_index << std::endl;

	// 相手プレイヤーを特定
	Player* opponentPlayer = (m_local_player_index == 0) ? &m_player2 : &m_player1;

	std::cout << "[Network] Opponent player ID: " << opponentPlayer->getId() << std::endl;
	std::cout << "[Network] Opponent player hand size: " << opponentPlayer->getHand().size() << std::endl;

	// 相手プレイヤーの手札が空の場合は警告を出すが、処理は続行
	// （デッキ選択後に手札が追加されるため）
	if (opponentPlayer->getHand().empty())
	{
		std::cout << "[Network] WARNING: Opponent's hand is empty! (this might be normal after deck draw)" << std::endl;
		// returnしない - 以前の変更でここでreturnしていたのが問題かも
	}

	if (!isSpecialCard)
	{
		// 通常カード: cardIdから色と値を復元
		int color = cardId / 100;
		int value = cardId % 100;

		std::cout << "[Network] Searching for card: color=" << color << ", value=" << value << std::endl;

		// 相手プレイヤーの手札を表示
		const auto& opponentHand = opponentPlayer->getHand();
		std::cout << "[Network] Opponent's hand: ";
		for (int i = 0; i < opponentHand.size(); i++)
		{
			if (auto cardPtr = std::dynamic_pointer_cast<Card>(opponentHand[i]))
			{
				std::cout << "[" << cardPtr->getColor() << "," << cardPtr->getValue() << "] ";
			}
		}
		std::cout << std::endl;

		// 相手プレイヤーの手札から該当するカードを検索
		int foundIndex = -1;
		for (int i = 0; i < opponentHand.size(); i++)
		{
			if (auto cardPtr = std::dynamic_pointer_cast<Card>(opponentHand[i]))
			{
				if (cardPtr->getColor() == color && cardPtr->getValue() == value)
				{
					foundIndex = i;
					std::cout << "[Network] Found card at index " << i << std::endl;
					break;
				}
			}
		}

		if (foundIndex >= 0)
		{
			// 手札から削除
			auto removedCard = opponentPlayer->removeCardFromHandByIndex(foundIndex);
			if (auto cardPtr = std::dynamic_pointer_cast<Card>(removedCard))
			{
				Slot& currentSlot = getSlot(flagIndex);
				currentSlot.placeCard(*this, *cardPtr, opponentPlayer);
				std::cout << "[Network] SUCCESS: Placed opponent's card (color=" << color << ", value=" << value << ") at flag " << flagIndex << std::endl;
			}
		}
		else
		{
			std::cout << "[Network] WARNING: Card NOT FOUND in opponent's hand - trying to place any card for visual sync (color=" << color << ", value=" << value << ")" << std::endl;
			// 手札から見つからない場合でも、手札の最初のカードを使用して配置
			// これにより、視覚的には相手がカードを配置したように見える（正確なカードではないが枚数は合う）
			if (opponentHand.size() > 0)
			{
				// 最初の通常カードを探して配置
				for (int i = 0; i < opponentHand.size(); i++)
				{
					if (auto cardPtr = std::dynamic_pointer_cast<Card>(opponentHand[i]))
					{
						auto removedCard = opponentPlayer->removeCardFromHandByIndex(i);
						if (auto card = std::dynamic_pointer_cast<Card>(removedCard))
						{
							Slot& currentSlot = getSlot(flagIndex);
							currentSlot.placeCard(*this, *card, opponentPlayer);
							std::cout << "[Network] SUCCESS: Placed alternative card (actual: color=" << card->getColor() << ", value=" << card->getValue() << ") for requested (color=" << color << ", value=" << value << ") at flag " << flagIndex << std::endl;
							break;
						}
					}
				}
			}
			else
			{
				std::cout << "[Network] ERROR: Opponent's hand is empty, cannot place card" << std::endl;
			}
		}
	}
	else
	{
		// 特殊カード: specialCardTypeから該当するカードを検索
		int foundIndex = -1;
		const auto& opponentHand = opponentPlayer->getHand();

		std::cout << "[Network] Searching for special card type=" << specialCardType << " in opponent's hand:" << std::endl;
		for (int i = 0; i < opponentHand.size(); i++)
		{
			if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(opponentHand[i]))
			{
				std::cout << "[Network]   Index " << i << ": SpecialCard type=" << static_cast<int>(specialCardPtr->getType()) << std::endl;
				if (static_cast<int>(specialCardPtr->getType()) == specialCardType)
				{
					foundIndex = i;
					std::cout << "[Network] MATCH FOUND at index " << i << std::endl;
					break;
				}
			}
			else if (auto cardPtr = std::dynamic_pointer_cast<Card>(opponentHand[i]))
			{
				std::cout << "[Network]   Index " << i << ": Normal Card (color=" << cardPtr->getColor() << ", value=" << cardPtr->getValue() << ")" << std::endl;
			}
		}

		if (foundIndex >= 0)
		{
			auto removedCard = opponentPlayer->removeCardFromHandByIndex(foundIndex);
			if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(removedCard))
			{
				SpecialCardCategory category = specialCardPtr->getCategory();

				if (category == ste_TroopCard)
				{
					Slot& currentSlot = getSlot(flagIndex);
					currentSlot.placeSpecialCard(*this, *specialCardPtr, opponentPlayer);
					std::cout << "[Network] SUCCESS: Placed opponent's troop card (type=" << static_cast<int>(specialCardPtr->getType()) << ") at flag " << flagIndex << std::endl;
				}
				else if (category == ste_WeatherTacticCard)
				{
					WeatherSlot& weatherSlot = getWeatherSlot(flagIndex);
					weatherSlot.placeWeatherCard(*this, *specialCardPtr, opponentPlayer);
					std::cout << "[Network] SUCCESS: Placed opponent's weather card at flag " << flagIndex << std::endl;
				}
				else if (category == ste_ConspiracyTacticCard)
				{
					ConspiracySlot& conspiracySlot = getConspiracySlot();
					conspiracySlot.placeConspiracyCard(*this, *specialCardPtr, opponentPlayer);
					std::cout << "[Network] SUCCESS: Placed opponent's conspiracy card" << std::endl;
				}

					// 特殊カード使用フラグを更新
				// 相手がカードを置いたので、相手は使えなくなり、自分（ローカルプレイヤー）が使えるようになる
				Player* localPlayer = (m_local_player_index == 0) ? &m_player1 : &m_player2;
				opponentPlayer->setCanUseSpecialCard(false);
				localPlayer->setCanUseSpecialCard(true);

				std::cout << "[Network] Flag updated after receiving special card placement" << std::endl;
				std::cout << "[Network]   Opponent (placed card, player " << opponentPlayer->getId() << "): canUse=" << opponentPlayer->getCanUseSpecialCard() << std::endl;
				std::cout << "[Network]   Local player (me, player " << localPlayer->getId() << "): canUse=" << localPlayer->getCanUseSpecialCard() << std::endl;
			}
		}
		else
		{
			std::cout << "[Network] WARNING: Special card NOT FOUND in opponent's hand - trying to place any special card for visual sync (type=" << specialCardType << ")" << std::endl;
			// 手札から見つからない場合でも、手札の最初の特殊カードを使用して配置
			if (opponentHand.size() > 0)
			{
				// 最初の特殊カードを探して配置
				for (int i = 0; i < opponentHand.size(); i++)
				{
					if (auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(opponentHand[i]))
					{
						auto removedCard = opponentPlayer->removeCardFromHandByIndex(i);
						if (auto card = std::dynamic_pointer_cast<SpecialCard>(removedCard))
						{
							SpecialCardCategory category = card->getCategory();

							if (category == ste_TroopCard)
							{
								Slot& currentSlot = getSlot(flagIndex);
								currentSlot.placeSpecialCard(*this, *card, opponentPlayer);
								std::cout << "[Network] SUCCESS: Placed alternative troop card at flag " << flagIndex << std::endl;
							}
							else if (category == ste_WeatherTacticCard)
							{
								WeatherSlot& weatherSlot = getWeatherSlot(flagIndex);
								weatherSlot.placeWeatherCard(*this, *card, opponentPlayer);
								std::cout << "[Network] SUCCESS: Placed alternative weather card at flag " << flagIndex << std::endl;
							}
							else if (category == ste_ConspiracyTacticCard)
							{
								ConspiracySlot& conspiracySlot = getConspiracySlot();
								conspiracySlot.placeConspiracyCard(*this, *card, opponentPlayer);
								std::cout << "[Network] SUCCESS: Placed alternative conspiracy card" << std::endl;
							}

							// 特殊カード使用フラグを更新
							opponentPlayer->setCanUseSpecialCard(false);
							getCurrentPlayer()->setCanUseSpecialCard(true);
							break;
						}
					}
				}
			}
			else
			{
				std::cout << "[Network] ERROR: Opponent's hand is empty, cannot place special card" << std::endl;
			}
		}
	}

	// デッキ選択待ち状態に設定（両方のクライアントで同じ状態にする）
	m_waiting_for_deck_choice = true;
	std::cout << "[Network] Card placed successfully, waiting for deck choice" << std::endl;
}

void GameState::onDeckChoiceReceived(int deckType)
{
	std::cout << "[Network] >>> Received DECK_CHOICE from remote: type=" << deckType << std::endl;
	std::cout << "[Network] This is the REMOTE client processing opponent's action" << std::endl;

	// ターン中のプレイヤー（相手側でカードを引いたプレイヤー）
	Player* playerWhoNeedsCard = m_currentPlayer;

	std::cout << "[Network] Current player (who drew card): " << playerWhoNeedsCard->getId() << std::endl;
	std::cout << "[Network] Player hand size before sync: " << playerWhoNeedsCard->getHand().size() << std::endl;
	std::cout << "[Network] Deck size before sync: normal=" << m_deck.getCards().size()
	          << ", special=" << m_special_deck.getCards().size() << std::endl;

	if (deckType == 0)
	{
		// 通常デッキから引く（リモート側でも手札に追加して同期）
		playerWhoNeedsCard->drawCard(&m_deck);
		std::cout << "[Network] Remote player " << playerWhoNeedsCard->getId() << " drew from normal deck" << std::endl;
		std::cout << "[Network] Deck remaining: " << m_deck.getCards().size() << std::endl;
	}
	else if (deckType == 1)
	{
		// 特殊デッキから引く（リモート側でも手札に追加して同期）
		playerWhoNeedsCard->drawSpecialCard(&m_special_deck);
		std::cout << "[Network] Remote player " << playerWhoNeedsCard->getId() << " drew from special deck" << std::endl;
		std::cout << "[Network] Special deck remaining: " << m_special_deck.getCards().size() << std::endl;
	}

	std::cout << "[Network] Player hand size after sync: " << playerWhoNeedsCard->getHand().size() << std::endl;

	// デッキ選択待ち状態を解除
	m_waiting_for_deck_choice = false;

	// ReconMode中でなければターン交代
	if (!m_recon_mode)
	{
		changePlayer();
	}
	else
	{
		std::cout << "[Network] ReconMode active, not changing player" << std::endl;
	}

	std::cout << "[Network] Turn changed, current player index: " << (m_currentPlayer == &m_player1 ? 0 : 1) << std::endl;
}

void GameState::onGameInitReceived(uint32_t seed, int hostPlayerIndex)
{
	std::cout << "[Network] Received GAME_INIT: seed=" << seed << ", host=" << hostPlayerIndex << std::endl;

	m_game_seed = seed;

	// ゲスト側でもホストと同じシードを使ってデッキをシャッフル
	std::cout << "[Network] Guest: Shuffling decks with seed: " << seed << std::endl;
	m_deck.shuffleWithSeed(seed);
	m_special_deck.shuffleWithSeed(seed + 1);

	// 両プレイヤーに初期手札を配る（7枚ずつ）
	// ホストと同じ順序でカードを引くため、同じシードなら同じ手札になる
	for (int i = 0; i < 7; i++) {
		m_player1.drawCard(&m_deck);
		m_player2.drawCard(&m_deck);
	}

	std::cout << "[Network] Guest: Dealt initial hands" << std::endl;
	std::cout << "[Network] Player1 hand size: " << m_player1.getHand().size() << std::endl;
	std::cout << "[Network] Player2 hand size: " << m_player2.getHand().size() << std::endl;
	std::cout << "[Network] Deck size: " << m_deck.getCards().size() << std::endl;
}

void GameState::onPlayerReadyReceived()
{
	std::cout << "[Network] Received PLAYER_READY" << std::endl;

	m_waiting_for_opponent = false;
}

void GameState::onDeckSyncReceived(const s3d::Array<int32>& deckOrder)
{
	std::cout << "[Network] >>> Received DECK_SYNC: " << deckOrder.size() << " cards" << std::endl;

	// デッキを受信した順序で復元
	m_deck.deserializeDeck(deckOrder);

	std::cout << "[Network] Normal deck synchronized successfully" << std::endl;
	std::cout << "[Network] Deck size: " << m_deck.getCards().size() << std::endl;
}

void GameState::onSpecialDeckSyncReceived(const s3d::Array<int32>& deckOrder)
{
	std::cout << "[Network] >>> Received SPECIAL_DECK_SYNC: " << deckOrder.size() << " cards" << std::endl;
	std::cout << "[Network] Card order received: ";
	for (int i = 0; i < deckOrder.size() && i < 10; i++) {
		std::cout << deckOrder[i] << " ";
	}
	std::cout << "..." << std::endl;

	// 特殊デッキを受信した順序で復元
	m_special_deck.deserializeDeck(deckOrder);

	std::cout << "[Network] Special deck synchronized successfully" << std::endl;
	std::cout << "[Network] Special deck size: " << m_special_deck.getCards().size() << std::endl;

	// 同期後のデッキ内容を確認
	const auto& cards = m_special_deck.getCards();
	std::cout << "[Network] Special deck contents (first 10): ";
	for (int i = 0; i < cards.size() && i < 10; i++) {
		std::cout << "type=" << static_cast<int>(cards[i].getType()) << " ";
	}
	std::cout << std::endl;
}

// ========================================
// スペシャルカードのネットワーク同期
// ========================================

void GameState::sendDeploymentCardAction(int sourceFlagIndex, int sourceSlotIndex, int targetFlagIndex, int targetSlotIndex)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending DEPLOYMENT_CARD_ACTION: source=(" << sourceFlagIndex << "," << sourceSlotIndex
	          << "), target=(" << targetFlagIndex << "," << targetSlotIndex << ")" << std::endl;

	s3d::Array<int32> data = { sourceFlagIndex, sourceSlotIndex, targetFlagIndex, targetSlotIndex };
	m_network->sendEvent(EVENT_DEPLOYMENT_CARD_ACTION, data);
}

void GameState::sendEscapeCardAction(int targetFlagIndex, int targetSlotIndex)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending ESCAPE_CARD_ACTION: target=(" << targetFlagIndex << "," << targetSlotIndex << ")" << std::endl;

	s3d::Array<int32> data = { targetFlagIndex, targetSlotIndex };
	m_network->sendEvent(EVENT_ESCAPE_CARD_ACTION, data);
}

void GameState::sendBetrayalCardAction(int sourceFlagIndex, int sourceSlotIndex, int destFlagIndex, int destSlotIndex)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending BETRAYAL_CARD_ACTION: source=(" << sourceFlagIndex << "," << sourceSlotIndex
	          << "), dest=(" << destFlagIndex << "," << destSlotIndex << ")" << std::endl;

	s3d::Array<int32> data = { sourceFlagIndex, sourceSlotIndex, destFlagIndex, destSlotIndex };
	m_network->sendEvent(EVENT_BETRAYAL_CARD_ACTION, data);
}

void GameState::sendReconCardAction(int phase, const s3d::Array<int32>& data)
{
	if (!m_network || !m_is_multiplayer)
		return;

	std::cout << "[Network] <<< Sending RECON_CARD_ACTION: phase=" << phase << ", data size=" << data.size() << std::endl;

	// phaseを先頭に追加
	s3d::Array<int32> fullData = { phase };
	fullData.append(data);
	m_network->sendEvent(EVENT_RECON_CARD_ACTION, fullData);
}

void GameState::onDeploymentCardActionReceived(const s3d::Array<int32>& data)
{
	if (data.size() < 4) {
		std::cout << "[Network] ERROR: Invalid DEPLOYMENT_CARD_ACTION data size" << std::endl;
		return;
	}

	int sourceFlagIndex = data[0];
	int sourceSlotIndex = data[1];
	int targetFlagIndex = data[2];
	int targetSlotIndex = data[3];

	std::cout << "[Network] >>> Received DEPLOYMENT_CARD_ACTION: source=(" << sourceFlagIndex << "," << sourceSlotIndex
	          << "), target=(" << targetFlagIndex << "," << targetSlotIndex << ")" << std::endl;

	// DeploymentCardの効果を適用
	// 注: 相手側の視点で処理するため、プレイヤーを反転
	Player* actingPlayer = (m_local_player_index == 0) ? &m_player2 : &m_player1;
	int actingPlayerId = actingPlayer->getId();

	Slot& sourceSlot = getSlot(sourceFlagIndex);
	auto removedCard = sourceSlot.getCards()[actingPlayerId][sourceSlotIndex];
	sourceSlot.getCards()[actingPlayerId][sourceSlotIndex] = nullptr;

	if (targetFlagIndex == -1) {
		// 削除の場合
		std::cout << "[Network] Card removed from flag " << sourceFlagIndex << std::endl;
	} else {
		// 移動の場合
		Slot& targetSlot = getSlot(targetFlagIndex);
		if (removedCard) {
			targetSlot.getCards()[actingPlayerId][targetSlotIndex] = removedCard;
			std::cout << "[Network] Card moved from flag " << sourceFlagIndex << " to flag " << targetFlagIndex << std::endl;
		}
	}
}

void GameState::onEscapeCardActionReceived(const s3d::Array<int32>& data)
{
	if (data.size() < 2) {
		std::cout << "[Network] ERROR: Invalid ESCAPE_CARD_ACTION data size" << std::endl;
		return;
	}

	int targetFlagIndex = data[0];
	int targetSlotIndex = data[1];

	std::cout << "[Network] >>> Received ESCAPE_CARD_ACTION: target=(" << targetFlagIndex << "," << targetSlotIndex << ")" << std::endl;

	// 相手が自分のカードを削除する
	Player* targetPlayer = (m_local_player_index == 0) ? &m_player1 : &m_player2;
	int targetPlayerId = targetPlayer->getId();

	Slot& targetSlot = getSlot(targetFlagIndex);
	targetSlot.getCards()[targetPlayerId][targetSlotIndex] = nullptr;

	std::cout << "[Network] Card escaped from flag " << targetFlagIndex << std::endl;
}

void GameState::onBetrayalCardActionReceived(const s3d::Array<int32>& data)
{
	if (data.size() < 4) {
		std::cout << "[Network] ERROR: Invalid BETRAYAL_CARD_ACTION data size" << std::endl;
		return;
	}

	int sourceFlagIndex = data[0];
	int sourceSlotIndex = data[1];
	int destFlagIndex = data[2];
	int destSlotIndex = data[3];

	std::cout << "[Network] >>> Received BETRAYAL_CARD_ACTION: source=(" << sourceFlagIndex << "," << sourceSlotIndex
	          << "), dest=(" << destFlagIndex << "," << destSlotIndex << ")" << std::endl;

	// 相手が自分のカードを奪って移動
	Player* sourcePlayer = (m_local_player_index == 0) ? &m_player1 : &m_player2;  // 奪われる側(自分)
	Player* destPlayer = (m_local_player_index == 0) ? &m_player2 : &m_player1;    // 奪う側(相手)
	int sourcePlayerId = sourcePlayer->getId();
	int destPlayerId = destPlayer->getId();

	Slot& sourceSlot = getSlot(sourceFlagIndex);
	auto removedCard = sourceSlot.getCards()[sourcePlayerId][sourceSlotIndex];
	sourceSlot.getCards()[sourcePlayerId][sourceSlotIndex] = nullptr;

	Slot& destSlot = getSlot(destFlagIndex);
	if (removedCard) {
		destSlot.getCards()[destPlayerId][destSlotIndex] = removedCard;
		std::cout << "[Network] Card betrayed from flag " << sourceFlagIndex << " to flag " << destFlagIndex << std::endl;
	}
}

void GameState::onReconCardActionReceived(const s3d::Array<int32>& data)
{
	if (data.size() < 1) {
		std::cout << "[Network] ERROR: Invalid RECON_CARD_ACTION data size" << std::endl;
		return;
	}

	int phase = data[0];
	std::cout << "[Network] >>> Received RECON_CARD_ACTION: phase=" << phase << std::endl;

	if (phase == 0)
	{
		// Phase 0->2の遷移: プレイヤーが3枚のカードを山札から手札に引いた
		// データ構造: [phase=0, drawnCard1ID, drawnCard2ID, drawnCard3ID]

		if (data.size() != 4)
		{
			std::cout << "[Network] ERROR: Invalid RECON_CARD_ACTION data size for phase 0: " << data.size() << std::endl;
			return;
		}

		int32 card1Id = data[1];
		int32 card2Id = data[2];
		int32 card3Id = data[3];

		std::cout << "[Network] Player drew 3 cards from deck: " << card1Id << ", " << card2Id << ", " << card3Id << std::endl;

		// 相手プレイヤー（カードを引いたプレイヤー）を取得
		// ReconCardを使ったのは相手なので、opponentPlayerを特定
		Player* actingPlayer = (m_local_player_index == 0) ? &m_player2 : &m_player1;

		std::cout << "[Network] Acting player (who used ReconCard): Player " << actingPlayer->getId() << std::endl;

		// 山札から該当するカードを探して削除し、手札に追加
		std::vector<std::shared_ptr<CardBase>> drawnCards;
		s3d::Array<int32> cardIdsToFind = { card1Id, card2Id, card3Id };

		// 通常デッキから探す
		const auto& deckCards = m_deck.getCards();
		for (size_t i = 0; i < deckCards.size() && drawnCards.size() < 3; ++i)
		{
			int32 cardId = deckCards[i].getId();
			for (int32 targetId : cardIdsToFind)
			{
				if (cardId == targetId)
				{
					// このカードを削除して手札に追加
					auto drawnCard = m_deck.removeCard(i);
					if (drawnCard)
					{
						drawnCards.push_back(drawnCard);
						std::cout << "[Network] Found and removed Card (ID=" << cardId << ") from normal deck at index " << i << std::endl;
					}
					break;
				}
			}
		}

		// 特殊デッキから探す
		const auto& specialDeckCards = m_special_deck.getCards();
		for (size_t i = 0; i < specialDeckCards.size() && drawnCards.size() < 3; ++i)
		{
			int32 cardId = specialDeckCards[i].getId();
			for (int32 targetId : cardIdsToFind)
			{
				if (cardId == targetId)
				{
					// このカードを削除して手札に追加
					auto drawnCard = m_special_deck.removeCard(i);
					if (drawnCard)
					{
						drawnCards.push_back(drawnCard);
						std::cout << "[Network] Found and removed SpecialCard (ID=" << cardId << ") from special deck at index " << i << std::endl;
					}
					break;
				}
			}
		}

		if (drawnCards.size() != 3)
		{
			std::cout << "[Network] WARNING: Could not find all 3 drawn cards in decks. Found: " << drawnCards.size() << std::endl;
		}

		// 引いたカードを相手の手札に追加（force=trueで上限を無視）
		for (auto& card : drawnCards)
		{
			actingPlayer->addCardToHand(card, true);
		}
		actingPlayer->update();

		std::cout << "[Network] ReconCard phase 0 complete. Player drew 3 cards and moved to phase 2." << std::endl;
	}
	else if (phase == 2)
	{
		// Phase 2完了: プレイヤーが2枚のカードを山札に戻した
		// データ構造: [phase=2, returnedCard1ID, returnedCard2ID]

		if (data.size() != 3)
		{
			std::cout << "[Network] ERROR: Invalid RECON_CARD_ACTION data size for phase 2: " << data.size() << std::endl;
			return;
		}

		int32 card1Id = data[1];
		int32 card2Id = data[2];

		std::cout << "[Network] Player returned 2 cards to deck: " << card1Id << ", " << card2Id << std::endl;

		// 相手プレイヤー（カードを返したプレイヤー）を取得
		// ReconCardを使ったのは相手なので、opponentPlayerを特定
		Player* actingPlayer = (m_local_player_index == 0) ? &m_player2 : &m_player1;
		auto& opponentHand = actingPlayer->getHand();

		std::cout << "[Network] Acting player (who used ReconCard): Player " << actingPlayer->getId() << std::endl;

		// 返されたカードを相手の手札から探して削除
		std::vector<int> indicesToRemove;
		std::vector<std::shared_ptr<CardBase>> cardsToReturn;

		for (int i = 0; i < opponentHand.size(); ++i)
		{
			if (opponentHand[i] && opponentHand[i]->getId() == card1Id)
			{
				std::cout << "[Network] Found card1 (ID=" << card1Id << ") at hand index " << i << std::endl;
				cardsToReturn.push_back(opponentHand[i]);
				indicesToRemove.push_back(i);
			}
			else if (opponentHand[i] && opponentHand[i]->getId() == card2Id)
			{
				std::cout << "[Network] Found card2 (ID=" << card2Id << ") at hand index " << i << std::endl;
				cardsToReturn.push_back(opponentHand[i]);
				indicesToRemove.push_back(i);
			}
		}

		if (cardsToReturn.size() != 2)
		{
			std::cout << "[Network] WARNING: Could not find both returned cards in opponent's hand. Found: " << cardsToReturn.size() << std::endl;
		}

		// インデックスの大きい方から削除
		std::sort(indicesToRemove.rbegin(), indicesToRemove.rend());
		for (int index : indicesToRemove)
		{
			actingPlayer->removeCardFromHandByIndex(index);
		}

		// カードをそれぞれの山札に戻す
		for (const auto& card : cardsToReturn)
		{
			if (std::dynamic_pointer_cast<SpecialCard>(card))
			{
				m_special_deck.returnCard(card);
				std::cout << "[Network] Returned SpecialCard (ID=" << card->getId() << ") to special deck" << std::endl;
			}
			else
			{
				m_deck.returnCard(card);
				std::cout << "[Network] Returned Card (ID=" << card->getId() << ") to normal deck" << std::endl;
			}
		}

		// ReconModeを終了し、山札選択をスキップして次のプレイヤーに交代
		// （こちら側では既に相手のターンなので、changePlayer()は不要）
		std::cout << "[Network] ReconCard phase 2 complete. Waiting for opponent's deck choice." << std::endl;
	}
	else
	{
		std::cout << "[Network] ReconCard phase " << phase << " synchronization not implemented" << std::endl;
	}
}

