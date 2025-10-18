#include "core/GameState.h"
#include "core/Common.h"
int checkSerialValue(int playerindex,int flagindex,std::vector<Flag>& m_flags,int serial_count=0);

GameState::GameState(Player player1, Player player2, Deck deck)
    : m_player1(player1), m_player2(player2), m_deck(deck)
{
    m_flags.reserve(ste_FlagMakeNum);
    for(int i=ste_FlagMinNum;i<=ste_FlagMaxNum;++i)
    {
        m_flags.emplace_back(i);
    }
}

Player* GameState::getPlayer1() { return &m_player1; }

Player* GameState::getPlayer2() { return &m_player2; }

Deck* GameState::getDeck() { return &m_deck; }

std::vector<Flag>& GameState::getFlags() { return m_flags; }

void GameState::setCurrentPlayer(Player* player) { m_currentPlayer = player; }

Player* GameState::getCurrentPlayer() const { return m_currentPlayer; }

bool GameState::getFinished() { return finished; }

void GameState::autoSetFinished()
{
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
    for(int i=ste_PlayerMin;i<=ste_PlayerMax;++i)
    {
        m_player_score[i]= ste_Flag_NonStatus;
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
