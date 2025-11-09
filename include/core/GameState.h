#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <Siv3D.hpp>
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
#include "NetworkEvents.h"

class SpecialCard;

namespace s3d {
	class Multiplayer_Photon;
}

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
        int m_recon_phase = 0; // 0: 山札選択, 1: カード一覧から選択, 2: 2枚戻す, 3: カードを山札に戻す表示
        std::vector<int> m_recon_selected_hand_indices; // Phase 2で選択した手札のインデックス
        bool m_recon_viewing_deck = false; // 現在山札一覧を表示中か
        bool m_recon_viewing_special = false; // 特殊デッキを表示中か
        std::vector<size_t> m_recon_selected_deck_indices; // Phase 1で選択した山札のインデックス
        double m_recon_scroll_offset = 0.0; // デッキ一覧のスクロールオフセット
        std::vector<std::shared_ptr<CardBase>> m_recon_cards_to_return; // Phase 3で戻すカード
        std::vector<bool> m_recon_return_to_special; // Phase 3で各カードが特殊デッキに戻るか

        // DeploymentCard (配置展開カード) 用の状態
        bool m_deployment_mode = false;
        int m_deployment_source_flag = -1;
        int m_deployment_source_slot = -1;

        // EscapeCard (脱走カード) 用の状態
        bool m_escape_mode = false;
        int m_escape_target_flag = -1;
        int m_escape_target_slot = -1;

        // BetrayalCard (裏切りカード) 用の状態
        bool m_betrayal_mode = false;
        int m_betrayal_source_flag = -1;
        int m_betrayal_source_slot = -1;

        // ネットワークマルチプレイ用の状態
        s3d::Multiplayer_Photon* m_network = nullptr;
        bool m_is_multiplayer = false;
        bool m_is_host = false;
        int m_local_player_index = 0;  // 0: Player1, 1: Player2
        bool m_waiting_for_opponent = false;
        uint32_t m_game_seed = 0;
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
        std::vector<size_t>& getReconSelectedDeckIndices() { return m_recon_selected_deck_indices; }
        void startReconCard();
		void setReconScrollOffset(double offset) { m_recon_scroll_offset = offset; };
		double getReconScrollOffset()  const { return m_recon_scroll_offset; }
		std::vector<std::shared_ptr<CardBase>>& getReconCardsToReturn() { return m_recon_cards_to_return; }
		std::vector<bool>& getReconReturnToSpecial() { return m_recon_return_to_special; }

        // DeploymentCard用のメソッド
        bool isDeploymentMode() const { return m_deployment_mode; }
        void setDeploymentMode(bool mode) { m_deployment_mode = mode; }
        int getDeploymentSourceFlag() const { return m_deployment_source_flag; }
        void setDeploymentSourceFlag(int flag) { m_deployment_source_flag = flag; }
        int getDeploymentSourceSlot() const { return m_deployment_source_slot; }
        void setDeploymentSourceSlot(int slot) { m_deployment_source_slot = slot; }
        void startDeploymentCard();

        // EscapeCard用のメソッド
        bool isEscapeMode() const { return m_escape_mode; }
        void setEscapeMode(bool mode) { m_escape_mode = mode; }
        int getEscapeTargetFlag() const { return m_escape_target_flag; }
        void setEscapeTargetFlag(int flag) { m_escape_target_flag = flag; }
        int getEscapeTargetSlot() const { return m_escape_target_slot; }
        void setEscapeTargetSlot(int slot) { m_escape_target_slot = slot; }
        void startEscapeCard();

        // BetrayalCard用のメソッド
        bool isBetrayalMode() const { return m_betrayal_mode; }
        void setBetrayalMode(bool mode) { m_betrayal_mode = mode; }
        int getBetrayalSourceFlag() const { return m_betrayal_source_flag; }
        void setBetrayalSourceFlag(int flag) { m_betrayal_source_flag = flag; }
        int getBetrayalSourceSlot() const { return m_betrayal_source_slot; }
        void setBetrayalSourceSlot(int slot) { m_betrayal_source_slot = slot; }
        void startBetrayalCard();

        // 特殊カード処理メソッド（UIロジック）
        void handleReconCard(const s3d::Font& instructionFont, const s3d::Font& smallFont, const HashTable<String, Vec2>& object_pos);
        void handleDeploymentCard(const s3d::Font& instructionFont);
        void handleEscapeCard(const s3d::Font& instructionFont);
        void handleBetrayalCard(const s3d::Font& instructionFont);

        // ネットワークマルチプレイ用メソッド
        void setNetwork(s3d::Multiplayer_Photon* network);
        bool isMultiplayer() const;
        bool isHost() const;
        bool isMyTurn() const;
        int getLocalPlayerIndex() const;
        void setMultiplayerMode(bool isMultiplayer, bool isHost);
        void setGameSeed(uint32_t seed);
        uint32_t getGameSeed() const;

        // ネットワークイベント送信メソッド
        void sendCardPlacedEvent(int flagIndex, int slotIndex, int cardId, bool isSpecialCard, int specialCardType);
        void sendDeckChoiceEvent(int deckType);
        void sendGameInitEvent(uint32_t seed);
        void sendPlayerReadyEvent();
        void sendDeckSyncEvent(const s3d::Array<int32>& deckOrder);
        void sendSpecialDeckSyncEvent(const s3d::Array<int32>& deckOrder);
        void sendDeploymentCardAction(int sourceFlagIndex, int sourceSlotIndex, int targetFlagIndex, int targetSlotIndex);
        void sendEscapeCardAction(int targetFlagIndex, int targetSlotIndex);
        void sendBetrayalCardAction(int sourceFlagIndex, int sourceSlotIndex, int destFlagIndex, int destSlotIndex);
        void sendReconCardAction(int phase, const s3d::Array<int32>& data);

        // ネットワークイベント受信処理メソッド
        void onCardPlacedReceived(int flagIndex, int slotIndex, int cardId, bool isSpecialCard, int specialCardType);
        void onDeckChoiceReceived(int deckType);
        void onGameInitReceived(uint32_t seed, int hostPlayerIndex);
        void onPlayerReadyReceived();
        void onDeckSyncReceived(const s3d::Array<int32>& deckOrder);
        void onSpecialDeckSyncReceived(const s3d::Array<int32>& deckOrder);
        void onDeploymentCardActionReceived(const s3d::Array<int32>& data);
        void onEscapeCardActionReceived(const s3d::Array<int32>& data);
        void onBetrayalCardActionReceived(const s3d::Array<int32>& data);
        void onReconCardActionReceived(const s3d::Array<int32>& data);
};

#endif
