#include "core/GameState.h"
#include "core/Common.h"
#include "core/Flag.h"
#include "core/Slot.h"
#include "core/WeatherSlot.h"
#include "core/ConspiracySlot.h"
#include "core/SpecialCard.h"
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

	s3d::Array<int32> data = { flagIndex, slotIndex, cardId, isSpecialCard ? 1 : 0, specialCardType };
	m_network->sendEvent(EVENT_CARD_PLACED, data);

	std::cout << "[Network] Sent CARD_PLACED: flag=" << flagIndex << ", slot=" << slotIndex << ", card=" << cardId << std::endl;
}

void GameState::sendDeckChoiceEvent(int deckType)
{
	if (!m_network || !m_is_multiplayer)
		return;

	m_network->sendEvent(EVENT_DECK_CHOICE, static_cast<int32>(deckType));

	std::cout << "[Network] Sent DECK_CHOICE: type=" << deckType << std::endl;
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

// ========================================
// ネットワークイベント受信処理メソッド
// ========================================

void GameState::onCardPlacedReceived(int flagIndex, int slotIndex, int cardId, bool isSpecialCard, int specialCardType)
{
	std::cout << "[Network] Received CARD_PLACED: flag=" << flagIndex << ", slot=" << slotIndex << ", card=" << cardId << std::endl;

	// TODO: 実際のカード配置処理を実装
	// 相手プレイヤーのカード配置を反映する
	// この部分は後で実装します
}

void GameState::onDeckChoiceReceived(int deckType)
{
	std::cout << "[Network] Received DECK_CHOICE: type=" << deckType << std::endl;

	// TODO: 相手のデッキ選択を反映
	// 相手がカードを引いたことを表示する処理など
}

void GameState::onGameInitReceived(uint32_t seed, int hostPlayerIndex)
{
	std::cout << "[Network] Received GAME_INIT: seed=" << seed << ", host=" << hostPlayerIndex << std::endl;

	m_game_seed = seed;
	// TODO: シードを使ってデッキを初期化
	// この部分は次のフェーズで実装します
}

void GameState::onPlayerReadyReceived()
{
	std::cout << "[Network] Received PLAYER_READY" << std::endl;

	m_waiting_for_opponent = false;
}

