#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <iostream>
#include "Player.h"
#include "Deck.h"
#include "SpecialDeck.h"
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
        SpecialDeck m_special_deck;
        Player* m_currentPlayer;
		Player* m_opponentPlayer;
        std::vector<Flag> m_flags;
        bool finished = false;
        int m_winner = NONE;
        int m_player_score[2]={0,0};
		std::vector<Slot> m_slots;
        bool m_waiting_for_deck_choice = false;
    public:
        GameState(Player player1, Player player2, Deck deck, SpecialDeck specialDeck);

        Player* getPlayer1();
		Player* getPlayer2();
        Deck* getDeck();
        SpecialDeck* getSpecialDeck();
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

        bool isWaitingForDeckChoice() const;
        void setWaitingForDeckChoice(bool waiting);
        void drawFromNormalDeck();
        void drawFromSpecialDeck();
};

#endif
