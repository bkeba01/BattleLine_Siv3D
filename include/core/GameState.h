#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <iostream>
#include "Player.h"
#include "Deck.h"
#include "Flag.h"

class Slot;

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
		Player* m_opponentPlayer;
        std::vector<Flag> m_flags;
        bool finished = false;
        int m_winner = NONE;
        int m_player_score[2]={0,0};
		std::vector<Slot> m_slots;
    public:
        GameState(Player player1, Player player2, Deck deck);

        Player* getPlayer1();
		Player* getPlayer2();
        Deck* getDeck();
        std::vector<Flag>& getFlags();
        Slot& getSlot(int flagIndex);
        Flag& getFlag(int slotIndex);
        
        void setCurrentPlayer(Player* player);
	    Player* getCurrentPlayer()const;

        bool getFinished();
        void autoSetFinished();

        void setWinner(int winner);
        int getWinner();

		void changePlayer();
		Player* getOpponentPlayer() const;
};

#endif
