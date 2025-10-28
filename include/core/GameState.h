#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <iostream>
#include <memory>
#include "Player.h"
#include "Deck.h"
#include "SpecialDeck.h"
#include "Flag.h"
#include "Slot.h"
#include "WeatherSlot.h"
#include "ConspiracySlot.h"

class SpecialCard;

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
		std::vector<WeatherSlot> m_weatherSlots;
		ConspiracySlot m_conspiracySlot;
        bool m_waiting_for_deck_choice = false;

        // ReconCard (偵察カード) 用の状態
        bool m_recon_mode = false;
        std::vector<std::shared_ptr<CardBase>> m_recon_drawn_cards; // 選択したカード（最大3枚）
        std::vector<bool> m_recon_card_from_special; // 各カードが特殊デッキから来たかどうか
        int m_recon_phase = 0; // 0: 山札選択, 1: カード一覧から選択, 2: 2枚戻す
        std::vector<int> m_recon_selected_hand_indices; // Phase 2で選択した手札のインデックス
        bool m_recon_viewing_deck = false; // 現在山札一覧を表示中か
        bool m_recon_viewing_special = false; // 特殊デッキを表示中か

        // DeploymentCard (配置展開カード) 用の状態
        bool m_deployment_mode = false;
        int m_deployment_source_flag = -1;
        int m_deployment_source_slot = -1;
    public:
        GameState(Player player1, Player player2, Deck deck, SpecialDeck specialDeck);

        Player* getPlayer1();
		Player* getPlayer2();
        Deck* getDeck();
        SpecialDeck* getSpecialDeck();
        std::vector<Flag>& getFlags();
        Slot& getSlot(int flagIndex);
		WeatherSlot& getWeatherSlot(int flagIndex);
		ConspiracySlot& getConspiracySlot();
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

        // ReconCard用のメソッド
        bool isReconMode() const { return m_recon_mode; }
        void setReconMode(bool mode) { m_recon_mode = mode; }
        int getReconPhase() const { return m_recon_phase; }
        void setReconPhase(int phase) { m_recon_phase = phase; }
        std::vector<std::shared_ptr<CardBase>>& getReconDrawnCards() { return m_recon_drawn_cards; }
        std::vector<bool>& getReconCardFromSpecial() { return m_recon_card_from_special; }
        std::vector<int>& getReconSelectedHandIndices() { return m_recon_selected_hand_indices; }
        bool isReconViewingDeck() const { return m_recon_viewing_deck; }
        void setReconViewingDeck(bool viewing) { m_recon_viewing_deck = viewing; }
        bool isReconViewingSpecial() const { return m_recon_viewing_special; }
        void setReconViewingSpecial(bool viewing) { m_recon_viewing_special = viewing; }
        void startReconCard();

        // DeploymentCard用のメソッド
        bool isDeploymentMode() const { return m_deployment_mode; }
        void setDeploymentMode(bool mode) { m_deployment_mode = mode; }
        int getDeploymentSourceFlag() const { return m_deployment_source_flag; }
        void setDeploymentSourceFlag(int flag) { m_deployment_source_flag = flag; }
        int getDeploymentSourceSlot() const { return m_deployment_source_slot; }
        void setDeploymentSourceSlot(int slot) { m_deployment_source_slot = slot; }
        void startDeploymentCard();
};

#endif
