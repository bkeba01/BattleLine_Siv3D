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
	m_recon_viewing_deck = false;
	m_recon_viewing_special = false;
}

void GameState::startDeploymentCard()
{
	m_deployment_mode = true;
	m_deployment_source_flag = -1;
	m_deployment_source_slot = -1;
}

