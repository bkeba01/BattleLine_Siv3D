#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <iostream>
#include "Player.h"
#include "Deck.h"
#include "Flag.h"

enum {
    NONE = -1,
    PLAYER1 = 0,
    PLAYER2 = 1
};

class GameState {
    private:
        Player m_player1;
        Player m_player2;
        Deck m_deck;
        Player* m_currentPlayer;
        std::vector<Flag> m_flags;
        bool finished = false;
        int m_winner = NONE;
        int m_player_score[2]={0,0};
    public:
        GameState(Player player1, Player player2, Deck deck);

        Player* getPlayer1();
		Player* getPlayer2();
        Deck* getDeck();
        std::vector<Flag>& getFlags();

        void setCurrentPlayer(Player* player);
        Player* getCurrentPlayer()const;

        bool getFinished();
        void autoSetFinished();

        void setWinner(int winner);
        int getWinner();

		void changePlayer();
};

#endif
