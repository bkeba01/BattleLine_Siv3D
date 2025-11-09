#include <Siv3D.hpp>
#include <memory>
#include "PHOTON_APP_ID.SECRET"
#include "core/Card.h"
#include "core/Deck.h"
#include "core/SpecialDeck.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/Flag.h"
#include "core/Slot.h"
#include "Multiplayer_Photon.hpp"
#include "core/NetworkEvents.h"
 // Photon App IDをここに定義する場合

// ゲームモード
enum class GameMode {
	Menu,
	LocalPlay,
	MultiplayerLobby,
	Playing
};

// マルチプレイヤーの状態
enum class MultiplayerState {
	NotConnected,
	Connecting,
	InLobby,
	CreatingRoom,
	EnteringRoomCode,  // ルームコード入力中
	JoiningRoom,
	InRoom,
	WaitingForOpponent,
	Ready
};

std::unique_ptr<GameState> initializeGame(HashTable<String,Vec2> object_pos, bool isMultiplayer = false)
{
	RectF deck_card{ Arg::center((object_pos[U"card_slot_size"].x / 2) + 10,Scene::Height() / 2),object_pos[U"card_slot_size"].x,object_pos[U"card_slot_size"].y };
	Deck deck(Font{ 30, Typeface::Bold }, Texture{ U"🃏"_emoji },Texture{U"⚔"_emoji});
	deck.setRect(deck_card);

	// マルチプレイの場合はシャッフルしない（後でシード同期する）
	if (!isMultiplayer) {
		deck.shuffle();
	}

	// 特殊デッキの初期化（右端）
	RectF special_deck_card{
		Arg::center(
			Scene::Width() - (object_pos[U"card_slot_size"].x / 2) - 10,
			Scene::Height() / 2
		),
		object_pos[U"card_slot_size"].x,
		object_pos[U"card_slot_size"].y
	};
	SpecialDeck specialDeck(Font{ 30, Typeface::Bold }, Texture{ U"✨"_emoji }, Texture{U"🎴"_emoji},Font{ 12, Typeface::Bold });
	specialDeck.setRect(special_deck_card);

	// マルチプレイの場合はシャッフルしない（後でシード同期する）
	if (!isMultiplayer) {
		specialDeck.shuffle();
	}

	Player player1(0, deck, object_pos[U"card_hand_size"], object_pos[U"card_hand_space"]);
	Player player2(1, deck, object_pos[U"card_hand_size"], object_pos[U"card_opponent_hand_space"]);
	auto gameState = std::make_unique<GameState>(player1, player2, deck, specialDeck);
	gameState->setCurrentPlayer(gameState->getPlayer1());
	const Texture Flag_texture{ Image{ U"resources/img/ball_red.png" }.scaled(0.2) };

	for (int flag = 0; flag < 9; flag++)
	{
		gameState->getFlags()[flag].setTexture(Flag_texture);
		gameState->getSlot(flag).setCardSlotSize(object_pos[U"card_slot_size"]);
		float flag_between_size = (object_pos[U"flag_space_size"].x - 9 * gameState->getFlags()[flag].getTexture().width()) / 10;
		float flag_x = object_pos[U"card_slot_size"].x + flag_between_size + (gameState->getFlags()[flag].getTexture().width() / 2) + flag * (gameState->getFlags()[flag].getTexture().width() + flag_between_size);

		Vec2 flag_position = { flag_x, object_pos[U"flag_space_size"].y };
		gameState->getSlot(flag).setFlagInitPosition(flag_position);
		gameState->getFlags()[flag].setPos(flag_position);

		// WeatherSlotの初期化
		gameState->getWeatherSlot(flag).setCardSlotSize(object_pos[U"card_slot_size"]);
		gameState->getWeatherSlot(flag).setFlagInitPosition(flag_position);
	}

	// ConspiracySlotの初期化
	gameState->getConspiracySlot().setCardSlotSize(object_pos[U"card_slot_size"]);
	gameState->getConspiracySlot().setSpecialDeckPosition(gameState->getSpecialDeck()->getRect().pos);

	return gameState;
}
HashTable<String, Vec2> initializePos()
{
	Scene::SetBackground(ColorF{ 0.3, 0.6, 0.4 });

	// 複数インスタンス起動時にウィンドウが重ならないようにランダムにずらす
	const int instanceNum = Random(1, 99);
	Window::SetTitle(U"Battle Line - Instance #" + ToString(instanceNum));
	Window::Resize(1200, 900);

	const int offsetX = Random(-200, 200);
	const int offsetY = Random(-100, 100);
	Window::SetPos(Window::GetPos() + Point(offsetX, offsetY));


	const float card_hand_width = 120;
	const float card_hand_height = 180;
	const float card_slot_width = card_hand_width*0.7;
	const float card_slot_height = card_hand_height*0.7;
	const float card_hand_x_space = (Scene::Width() - (card_hand_width / 2 * 7)) / 2;
	const float player_hand_y = Scene::Height() * 0.9;
	const float opponent_hand_y = Scene::Height() * 0.1;
	const float flag_space_size_x = Scene::Width() - 2*(card_slot_width);
	const float flag_y = Scene::Height() / 2;
	HashTable<String, Vec2>object_pos = {
		{U"card_hand_size",Vec2(card_hand_width,card_hand_height)},
		{U"card_slot_size",Vec2(card_slot_width,card_slot_height)},
		{U"card_hand_space",Vec2(card_hand_x_space,player_hand_y)},
		{U"card_opponent_hand_space",Vec2(card_hand_x_space,opponent_hand_y)},
		{U"flag_space_size",Vec2(flag_space_size_x,flag_y)}
	};



	return object_pos;
}

// Photonイベントハンドラークラス
class GamePhotonHandler : public s3d::Multiplayer_Photon {
private:
	GameState* m_gameState = nullptr;

public:
	GamePhotonHandler(const std::string_view secretPhotonAppID, const StringView photonAppVersion)
		: Multiplayer_Photon(secretPhotonAppID, photonAppVersion, s3d::Verbose::Yes) {
		Print << U"[Photon] Initializing with App ID: " << Unicode::Widen(std::string(secretPhotonAppID));
	}

	void setGameState(GameState* gameState) {
		m_gameState = gameState;
	}

	// 接続エラーの詳細を表示
	void connectionErrorReturn(const int32 errorCode) override {
		Print << U"";
		Print << U"========================================";
		Print << U"[Photon ERROR] Connection failed!";
		Print << U"========================================";
		Print << U"Error Code: " << errorCode;
		Print << U"";

		// エラーコード別の詳細説明
		if (errorCode == 1043) {
			Print << U"エラー 1043: Invalid Authentication";
			Print << U"";
			Print << U"原因: 認証情報（App ID/Secret Key）が正しくありません";
			Print << U"";
			Print << U"解決方法:";
			Print << U"1. Photonダッシュボードを開く";
			Print << U"   https://dashboard.photonengine.com/";
			Print << U"";
			Print << U"2. Applications → あなたのアプリを選択";
			Print << U"";
			Print << U"3. 以下の2つを確認:";
			Print << U"   ・App ID (通常はこちら)";
			Print << U"   ・Secret Key (Showボタンで表示)";
			Print << U"";
			Print << U"4. まず App ID を試す:";
			Print << U"   Main.cpp:584 の行を以下のように変更:";
			Print << U"   const std::string PHOTON_APP_ID = \"あなたのApp ID\";";
			Print << U"";
			Print << U"5. App IDでダメなら Secret Key を試す:";
			Print << U"   Main.cpp:584 をコメントアウトして";
			Print << U"   Main.cpp:587 のコメントを外す";
			Print << U"";
			Print << U"6. 再ビルドして実行";
		} else {
			Print << U"考えられる原因:";
			Print << U"  1. Photon App ID/Secret Keyが正しく設定されていない";
			Print << U"  2. インターネット接続の問題";
			Print << U"  3. Photon Realtimeではなく別のタイプを選択している";
			Print << U"";
			Print << U"対処法:";
			Print << U"  - https://dashboard.photonengine.com/ で認証情報を確認";
			Print << U"  - Photon Type が 'Realtime' か確認";
			Print << U"  - Main.cpp:584 の設定を確認";
		}

		Print << U"========================================";
		Print << U"";

		// 親クラスのメソッドも呼ぶ
		Multiplayer_Photon::connectionErrorReturn(errorCode);
	}

	// 接続成功時の詳細を表示
	void connectReturn(const int32 errorCode, const String& errorString, const String& region, const String& cluster) override {
		if (errorCode) {
			Print << U"[Photon ERROR] Connect failed!";
			Print << U"  Error Code: " << errorCode;
			Print << U"  Error String: " << errorString;
		} else {
			Print << U"[Photon SUCCESS] Connected!";
			Print << U"  Region: " << region;
			Print << U"  Cluster: " << cluster;
		}

		// 親クラスのメソッドも呼ぶ
		Multiplayer_Photon::connectReturn(errorCode, errorString, region, cluster);
	}

	// int32版のオーバーロード（単一値用）
	void customEventAction(const s3d::LocalPlayerID playerID, const uint8 eventCode, int32 data) override {
		if (!m_gameState) {
			Multiplayer_Photon::customEventAction(playerID, eventCode, data);
			return;
		}

		// 自分が送信したイベントは既にローカルで処理済みなのでスキップ
		if (playerID == getLocalPlayerID()) {
			Multiplayer_Photon::customEventAction(playerID, eventCode, data);
			return;
		}

		switch (eventCode) {
		case EVENT_DECK_CHOICE:
			m_gameState->onDeckChoiceReceived(data);
			break;
		default:
			break;
		}

		Multiplayer_Photon::customEventAction(playerID, eventCode, data);
	}

	// イベント受信時のコールバック（配列版）
	void customEventAction(const s3d::LocalPlayerID playerID, const uint8 eventCode, const s3d::Array<int32>& data) override {
		if (!m_gameState) {
			Multiplayer_Photon::customEventAction(playerID, eventCode, data);
			return;
		}

		// 自分が送信したイベントは既にローカルで処理済みなのでスキップ
		if (playerID == getLocalPlayerID()) {
			Multiplayer_Photon::customEventAction(playerID, eventCode, data);
			return;
		}

		switch (eventCode) {
		case EVENT_GAME_INIT:
			if (data.size() >= 2) {
				uint32_t seed = static_cast<uint32_t>(data[0]);
				int hostPlayerIndex = data[1];
				m_gameState->onGameInitReceived(seed, hostPlayerIndex);
			}
			break;

		case EVENT_CARD_PLACED:
			if (data.size() >= 5) {
				int flagIndex = data[0];
				int slotIndex = data[1];
				int cardId = data[2];
				bool isSpecialCard = (data[3] != 0);
				int specialCardType = data[4];
				m_gameState->onCardPlacedReceived(flagIndex, slotIndex, cardId, isSpecialCard, specialCardType);
			}
			break;

		case EVENT_PLAYER_READY:
			m_gameState->onPlayerReadyReceived();
			break;

		case EVENT_DECK_SYNC:
			m_gameState->onDeckSyncReceived(data);
			break;

		case EVENT_SPECIAL_DECK_SYNC:
			m_gameState->onSpecialDeckSyncReceived(data);
			break;

		case EVENT_DEPLOYMENT_CARD_ACTION:
			m_gameState->onDeploymentCardActionReceived(data);
			break;

		case EVENT_ESCAPE_CARD_ACTION:
			m_gameState->onEscapeCardActionReceived(data);
			break;

		case EVENT_BETRAYAL_CARD_ACTION:
			m_gameState->onBetrayalCardActionReceived(data);
			break;

		case EVENT_RECON_CARD_ACTION:
			m_gameState->onReconCardActionReceived(data);
			break;

		default:
			break;
		}

		// 親クラスのメソッドも呼ぶ
		Multiplayer_Photon::customEventAction(playerID, eventCode, data);
	}
};

// メニュー画面の描画
GameMode drawMenu(Font& titleFont, Font& buttonFont) {
	Scene::SetBackground(ColorF{ 0.2, 0.4, 0.3 });

	// タイトル
	titleFont(U"Battle Line").drawAt(Scene::Center().x, 150, Palette::White);

	// ボタン
	const RectF localButton{ Arg::center(Scene::Center().x, 350), 300, 80 };
	const RectF multiButton{ Arg::center(Scene::Center().x, 450), 300, 80 };
	const RectF exitButton{ Arg::center(Scene::Center().x, 550), 300, 80 };

	// ローカルプレイボタン
	if (localButton.mouseOver()) {
		localButton.draw(ColorF{ 0.4, 0.7, 0.5 });
		Cursor::RequestStyle(CursorStyle::Hand);
		if (MouseL.down()) {
			return GameMode::LocalPlay;
		}
	}
	else {
		localButton.draw(ColorF{ 0.3, 0.6, 0.4 });
	}
	localButton.drawFrame(3, Palette::White);
	buttonFont(U"ローカルプレイ").drawAt(localButton.center(), Palette::White);

	// オンラインマルチプレイボタン
	if (multiButton.mouseOver()) {
		multiButton.draw(ColorF{ 0.5, 0.7, 0.8 });
		Cursor::RequestStyle(CursorStyle::Hand);
		if (MouseL.down()) {
			return GameMode::MultiplayerLobby;
		}
	}
	else {
		multiButton.draw(ColorF{ 0.4, 0.6, 0.7 });
	}
	multiButton.drawFrame(3, Palette::White);
	buttonFont(U"オンラインマルチプレイ").drawAt(multiButton.center(), Palette::White);

	// 終了ボタン
	if (exitButton.mouseOver()) {
		exitButton.draw(ColorF{ 0.7, 0.4, 0.4 });
		Cursor::RequestStyle(CursorStyle::Hand);
		if (MouseL.down()) {
			System::Exit();
		}
	}
	else {
		exitButton.draw(ColorF{ 0.6, 0.3, 0.3 });
	}
	exitButton.drawFrame(3, Palette::White);
	buttonFont(U"終了").drawAt(exitButton.center(), Palette::White);

	return GameMode::Menu;
}

// マルチプレイヤーロビー画面の描画
MultiplayerState drawMultiplayerLobby(Font& titleFont, Font& buttonFont, Font& infoFont,
	GamePhotonHandler& photon, MultiplayerState currentState, TextEditState& roomNameState,
	String& currentRoomCode, std::unique_ptr<GameState>& gameState) {  // ルームコードを追加

	Scene::SetBackground(ColorF{ 0.2, 0.3, 0.4 });

	titleFont(U"オンラインマルチプレイ").drawAt(Scene::Center().x, 100, Palette::White);

	// 接続状態の表示
	String stateText;
	switch (currentState) {
	case MultiplayerState::NotConnected:
		stateText = U"未接続";
		break;
	case MultiplayerState::Connecting:
		stateText = U"接続中...";
		break;
	case MultiplayerState::InLobby:
		stateText = U"ロビーに接続済み";
		break;
	case MultiplayerState::InRoom:
		stateText = U"ルームに参加: " + photon.getCurrentRoomName();
		break;
	case MultiplayerState::WaitingForOpponent:
		stateText = U"対戦相手を待っています...";
		break;
	case MultiplayerState::Ready:
		stateText = U"ゲーム開始準備完了！";
		break;
	default:
		stateText = U"不明";
		break;
	}
	infoFont(U"状態: " + stateText).drawAt(Scene::Center().x, 180, Palette::White);

	// プレイヤー情報
	if (currentState != MultiplayerState::NotConnected) {
		infoFont(U"プレイヤー名: " + photon.getUserName()).drawAt(Scene::Center().x, 220, Palette::White);
		infoFont(U"Ping: " + ToString(photon.getPingMillisec()) + U"ms").drawAt(Scene::Center().x, 250, Palette::White);
	}

	// ルームにいる場合、ルームコードと参加者を表示
	if (currentState == MultiplayerState::InRoom || currentState == MultiplayerState::WaitingForOpponent) {
		// ルームコードを大きく表示
		RectF codeBox{ Arg::center(Scene::Center().x, 320), 400, 120 };
		codeBox.draw(ColorF{ 0.3, 0.5, 0.7 });
		codeBox.drawFrame(4, Palette::White);
		infoFont(currentRoomCode).drawAt(codeBox.center(), Palette::White);

		infoFont(U"ルームコード（相手に伝えてください）").drawAt(Scene::Center().x, 240, Palette::Yellow);

		auto players = photon.getLocalPlayers();
		infoFont(U"参加者数: " + ToString(players.size()) + U" / 2").drawAt(Scene::Center().x, 460, Palette::White);

		int yOffset = 500;
		for (const auto& player : players) {
			String playerInfo = player.userName;
			if (player.isHost) playerInfo += U" [ホスト]";
			if (player.localID == photon.getLocalPlayerID()) playerInfo += U" (あなた)";
			infoFont(playerInfo).drawAt(Scene::Center().x, yOffset, Palette::White);
			yOffset += 30;
		}

		// 2人揃ったらゲーム開始
		if (players.size() == 2 && currentState != MultiplayerState::Ready) {
			return MultiplayerState::Ready;
		}
	}

	// ボタン配置
	const RectF connectButton{ Arg::center(Scene::Center().x, 450), 280, 60 };
	const RectF createRoomButton{ Arg::center(Scene::Center().x - 160, 530), 280, 60 };
	const RectF joinRoomButton{ Arg::center(Scene::Center().x + 160, 530), 280, 60 };
	const RectF backButton{ Arg::center(Scene::Center().x, 650), 200, 50 };

	// 接続ボタン
	if (currentState == MultiplayerState::NotConnected) {
		if (connectButton.mouseOver()) {
			connectButton.draw(ColorF{ 0.4, 0.7, 0.5 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				return MultiplayerState::Connecting;
			}
		}
		else {
			connectButton.draw(ColorF{ 0.3, 0.6, 0.4 });
		}
		connectButton.drawFrame(3, Palette::White);
		buttonFont(U"サーバーに接続").drawAt(connectButton.center(), Palette::White);
	}

	// ルーム作成ボタンと参加ボタン
	if (currentState == MultiplayerState::InLobby) {
		// ルーム作成ボタン
		if (createRoomButton.mouseOver()) {
			createRoomButton.draw(ColorF{ 0.5, 0.7, 0.8 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				return MultiplayerState::CreatingRoom;
			}
		}
		else {
			createRoomButton.draw(ColorF{ 0.4, 0.6, 0.7 });
		}
		createRoomButton.drawFrame(3, Palette::White);
		buttonFont(U"ルーム作成").drawAt(createRoomButton.center(), Palette::White);

		// ルームに参加ボタン
		if (joinRoomButton.mouseOver())
		{
			joinRoomButton.draw(ColorF{ 0.7, 0.6, 0.5 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				return MultiplayerState::EnteringRoomCode;
			}
		}
		else {
			joinRoomButton.draw(ColorF{ 0.6, 0.5, 0.4 });
		}
		joinRoomButton.drawFrame(3, Palette::White);
		buttonFont(U"ルームに参加").drawAt(joinRoomButton.center(), Palette::White);
	}

	// ルームコード入力画面
	if (currentState == MultiplayerState::EnteringRoomCode) {
		// 入力ボックスの描画
		//Font inputFont{ 32, Typeface::Bold };
		RectF inputBox{ Arg::center(Scene::Center().x, 350), 300, 80 };
		inputBox.draw(ColorF{ 0.2, 0.2, 0.3 });
		inputBox.drawFrame(4, Palette::White);

		infoFont(U"4桁のルームコードを入力してください").drawAt(Scene::Center().x, 280, Palette::White);

		// テキスト入力
		SimpleGUI::TextBox(roomNameState, Vec2{ Scene::Center().x - 100, 330 }, 200);

		// 入力値を4桁に制限
		if (roomNameState.text.size() > 4) {
			roomNameState.text = roomNameState.text.substr(0, 4);
		}

		// 数字のみに制限
		String filtered;
		for (auto ch : roomNameState.text) {
			if (ch >= U'0' && ch <= U'9') {
				filtered += ch;
			}
		}
		if (filtered != roomNameState.text) {
			roomNameState.text = filtered;
		}
		infoFont(U"入力中のルームコード: " + roomNameState.text).drawAt(Scene::Center().x, 400, Palette::White);
		 // ゲーム状態にルームコードを設定
		// 参加ボタン
		const RectF joinButton{ Arg::center(Scene::Center().x, 450), 200, 60 };
		bool canJoin = roomNameState.text.size() == 4;

		if (canJoin && joinButton.mouseOver()) {
			joinButton.draw(ColorF{ 0.5, 0.7, 0.5 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				currentRoomCode = roomNameState.text;
				return MultiplayerState::JoiningRoom;
			}
		}
		else {
			joinButton.draw(canJoin ? ColorF{ 0.4, 0.6, 0.4 } : ColorF{ 0.3, 0.3, 0.3 });
		}
		joinButton.drawFrame(3, Palette::White);
		buttonFont(U"参加").drawAt(joinButton.center(), canJoin ? ColorF{ 0.0, 0.0, 0.0 } : ColorF{ 0.5, 0.5, 0.5 });

		// キャンセルボタン
		const RectF cancelButton{ Arg::center(Scene::Center().x, 550), 200, 50 };
		if (cancelButton.mouseOver()) {
			cancelButton.draw(ColorF{ 0.6, 0.4, 0.4 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				roomNameState.clear();
				return MultiplayerState::InLobby;
			}
		}
		else {
			cancelButton.draw(ColorF{ 0.5, 0.3, 0.3 });
		}
		cancelButton.drawFrame(3, Palette::White);
		infoFont(U"キャンセル").drawAt(cancelButton.center(), Palette::White);
	}

	// 戻るボタン
	if (currentState == MultiplayerState::NotConnected || currentState == MultiplayerState::InLobby) {
		if (backButton.mouseOver()) {
			backButton.draw(ColorF{ 0.6, 0.4, 0.4 });
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) {
				if (photon.isActive()) {
					photon.disconnect();
				}
				// メニューに戻るための特別な状態を返す
				return MultiplayerState::NotConnected; // この後、メインループでGameMode::Menuに戻す
			}
		}
		else {
			backButton.draw(ColorF{ 0.5, 0.3, 0.3 });
		}
		backButton.drawFrame(3, Palette::White);
		buttonFont(U"戻る").drawAt(backButton.center(), Palette::White);
	}

	return currentState;
}

// ゲームプレイ部分（既存のMain関数の中身）
void runGame(HashTable<String, Vec2>& object_pos, std::unique_ptr<GameState>& gameState, GamePhotonHandler* photon = nullptr) {
	// Initial update to set card positions
	gameState->getPlayer1()->update();
	gameState->getPlayer2()->update();
	Font instructionFont{ 24, Typeface::Bold };
	Font smallFont{ 16, Typeface::Bold };
	Font debugFont{ 14, Typeface::Bold };
	while (System::Update())
	{
		// マルチプレイ時はPhotonの更新が必須
		if (photon) {
			photon->update();
		}

		gameState->autoSetFinished();
		if (gameState->getFinished())
		{
			break;
		}

		gameState->getDeck()->drawDeck();
		gameState->getSpecialDeck()->drawDeck(gameState->getCurrentPlayer()->getCanUseSpecialCard());

		// Draw special card usage indicator coin
		// 片方のプレイヤーだけが使用可能な場合のみ描画（最初は両方trueなので描画しない）
		Player* player1 = gameState->getPlayer1();
		Player* player2 = gameState->getPlayer2();
		bool player1CanUse = player1->getCanUseSpecialCard();
		bool player2CanUse = player2->getCanUseSpecialCard();

		if (player1CanUse != player2CanUse)
		{
			RectF deckRect = gameState->getDeck()->getRect();
			double coinDiameter = deckRect.w * 2.0 / 3.0;
			double coinX = deckRect.x + deckRect.w / 2.0;
			double coinY;

			// マルチプレイの場合は、ローカルプレイヤーの視点で判断
			bool showCoinBelow; // 下側（自分側）にコインを表示するか

			if (gameState->isMultiplayer())
			{
				int localIndex = gameState->getLocalPlayerIndex();
				Player* localPlayer = (localIndex == 0) ? player1 : player2;
				showCoinBelow = localPlayer->getCanUseSpecialCard();
			}
			else
			{
				// シングルプレイの場合は、現在のターンプレイヤーで判断
				showCoinBelow = gameState->getCurrentPlayer()->getCanUseSpecialCard();
			}

			if (showCoinBelow)
			{
				// Local player can use special card - show coin below deck (自分側)
				coinY = deckRect.y + deckRect.h + 60;
			}
			else
			{
				// Remote player can use special card - show coin above deck (相手側)
				coinY = deckRect.y - 60;
			}

			Circle(coinX, coinY, coinDiameter / 2.0).draw(Palette::Gold);
		}

		// Determine local/remote players for drawing
		Player* localPlayer;
		Player* remotePlayer;
		int localPlayerId;

		if (gameState->isMultiplayer()) {
			// マルチプレイヤーモード：ローカルプレイヤーを常に下側に表示
			int localIndex = gameState->getLocalPlayerIndex();
			localPlayer = (localIndex == 0) ? gameState->getPlayer1() : gameState->getPlayer2();
			remotePlayer = (localIndex == 0) ? gameState->getPlayer2() : gameState->getPlayer1();
			localPlayerId = localPlayer->getId();
		} else {
			// ローカルモード：現在のターンプレイヤーを下側に表示（従来の動作）
			localPlayer = gameState->getCurrentPlayer();
			remotePlayer = (localPlayer == gameState->getPlayer1()) ? gameState->getPlayer2() : gameState->getPlayer1();
			localPlayerId = localPlayer->getId();
		}

		// Get the current dragged card type to conditionally show empty slots
		DraggedCardType draggedType = gameState->getCurrentPlayer()->getDraggedCardType();

		// DeploymentCardモードでドラッグ中かチェック
		bool deploymentDragging = gameState->isDeploymentMode()
			&& gameState->getDeploymentSourceFlag() != -1
			&& MouseL.pressed();

		// BetrayalCardモードでドラッグ中かチェック
		bool betrayalDragging = gameState->isBetrayalMode()
			&& gameState->getBetrayalSourceFlag() != -1
			&& MouseL.pressed();

		for (int flag = 0; flag < 9; flag++)
		{
			// Always draw normal Slot (placed cards), show empty slots only when dragging NormalCard or TroopCard
			// または DeploymentCard でドラッグ中で、勝敗が決まっていないFlagの場合
			// または BetrayalCard でドラッグ中で、勝敗が決まっていないFlagの場合
			bool isUndecidedFlag = gameState->getFlags()[flag].getFlagStatus() == ste_NonePlayer;
			bool showNormalSlots = (draggedType == DraggedCardType::NormalCard || draggedType == DraggedCardType::TroopCard)
				|| (deploymentDragging && isUndecidedFlag && flag != gameState->getDeploymentSourceFlag())
				|| (betrayalDragging && isUndecidedFlag);
			gameState->getSlot(flag).slotdraw(*gameState, localPlayerId, showNormalSlots);

			// Always draw WeatherSlot (placed cards), show empty slots only when dragging WeatherCard
			bool showWeatherSlots = (draggedType == DraggedCardType::WeatherCard);
			gameState->getWeatherSlot(flag).slotdraw(*gameState, localPlayerId, showWeatherSlots);

			// Always draw flags
			if (gameState->getFlags()[flag].getFlagStatus() == ste_NonePlayer)
			{
				gameState->getFlags()[flag].draw();
			}
			else
			{
				gameState->getFlags()[flag].drawWinnerFlag(*gameState);
			}
		}

		// Always draw ConspiracySlot (placed cards), show empty slots only when dragging ConspiracyCard
		bool showConspiracySlots = (draggedType == DraggedCardType::ConspiracyCard);
		gameState->getConspiracySlot().slotdraw(*gameState, localPlayerId, showConspiracySlots);

		// Update visuals for both players
		gameState->getPlayer1()->update();
		gameState->getPlayer2()->update();

		// 特殊カードの処理（カプセル化されたメソッドを呼び出し）
		gameState->handleReconCard(instructionFont, smallFont, object_pos);
		gameState->handleDeploymentCard(instructionFont);
		gameState->handleEscapeCard(instructionFont);
		gameState->handleBetrayalCard(instructionFont);

		// 山札選択またはカード配置の入力処理
		if (gameState->isWaitingForDeckChoice())
		{
			// 視覚的フィードバック：指示テキスト表示
			instructionFont(U"山札を選んでカードを引いてください").drawAt(Scene::Center().movedBy(0, -250), Palette::Yellow);

			// 視覚的フィードバック：デッキをホバー時にハイライト
			if (gameState->getDeck()->getRect().mouseOver())
			{
				gameState->getDeck()->getRect().drawFrame(5, Palette::Yellow);
			}
			if (gameState->getSpecialDeck()->getRect().mouseOver())
			{
				gameState->getSpecialDeck()->getRect().drawFrame(5, Palette::Gold);
			}

			// 山札選択の入力処理
			gameState->getCurrentPlayer()->handleDeckChoice(*gameState);
		}
		else
		{
			// 通常のカード配置入力処理
			gameState->getCurrentPlayer()->handleInput(*gameState);
		}

		gameState->autoSetFinished();

		// Set hand positions for both players
		const Vec2 player_hand_pos = object_pos[U"card_hand_space"];
		const Vec2 opponent_hand_pos = { object_pos[U"card_hand_space"].x, Scene::Height() * 0.1 };

		localPlayer->setHandSpace(player_hand_pos);
		remotePlayer->setHandSpace(opponent_hand_pos);

		// Draw hands from the local player's perspective
		localPlayer->draw(*gameState);
		remotePlayer->drawBacks();

		// ReconModeのPhase 2では選択したカードに黄色のフレームを描画
		if (gameState->isReconMode() && gameState->getReconPhase() == 2)
		{
			auto& selectedIndices = gameState->getReconSelectedHandIndices();
			const auto& hand = localPlayer->getHand();
			for (int index : selectedIndices)
			{
				if (index >= 0 && index < static_cast<int>(hand.size()) && hand[index])
				{
					hand[index]->getRect().drawFrame(5, Palette::Yellow);
				}
			}
		}
	}
	const int winner = gameState->getWinner();
	const String winnerText = U"Winner: Player " + ToString(winner + 1);
	Font font{ 60, Typeface::Bold };
	while(System::Update())
	{
		Scene::SetBackground(ColorF{ 0.3, 0.6, 0.4 });
		font(winnerText).drawAt(Scene::Center(), Palette::White);
	}
}

// 新しいMain関数（メニューとマルチプレイヤー対応）
void Main()
{
	// フォントの準備
	Font titleFont{ 60, Typeface::Bold };
	Font buttonFont{ 32, Typeface::Bold };
	Font infoFont{ 20, Typeface::Regular };

	// ゲームモード
	GameMode currentMode = GameMode::Menu;
	MultiplayerState multiplayerState = MultiplayerState::NotConnected;

	// Photonクライアント（必要時に初期化）
	std::optional<GamePhotonHandler> photon;
	TextEditState roomNameState{ U"" };  // ルームコード入力用
	String currentRoomCode = U"";        // 現在のルームコード（4桁）

	// ゲームステート
	HashTable<String, Vec2> object_pos;
	std::unique_ptr<GameState> gameState;

	//std::string PHOTON_APP_ID;
	String PHOTON_APP_VERSION;


	// 設定ファイルの読み込み
	const TOMLReader config{ U"config.toml" };

	if (not config) {
		Console << U"";
		Console << U"⚠️ エラー: config.toml が見つかりません！";
		Console << U"";
		Console << U"設定方法:";
		Console << U"1. App/config.toml.example を App/config.toml にコピー";
		Console << U"2. https://dashboard.photonengine.com/ にアクセス";
		Console << U"3. Applications → あなたのアプリを選択";
		Console << U"4. App ID をコピー";
		Console << U"5. config.toml の app_id に貼り付け";
		Console << U"";
		Console << U"フォールバック: ハードコードされた値を使用します";

		// フォールバック値（設定ファイルがない場合）
		//PHOTON_APP_ID = "ab1b65a8sdrfdg";
		PHOTON_APP_VERSION = U"1.0";
	}
	else {
		// 設定ファイルから読み込み
		//PHOTON_APP_ID = config[U"photon.app_id"].getString().narrow();
		PHOTON_APP_VERSION = config[U"photon.app_version"].getOr<String>(U"1.0");

		Console << U"[Config] config.toml から設定を読み込みました";
	}



	while (System::Update())
	{
		switch (currentMode)
		{
		case GameMode::Menu:
		{
			GameMode newMode = drawMenu(titleFont, buttonFont);
			if (newMode != currentMode) {
				if (newMode == GameMode::LocalPlay) {
					// ローカルプレイモードへ
					object_pos = initializePos();
					gameState = initializeGame(object_pos);
					currentMode = GameMode::Playing;
				}
				else if (newMode == GameMode::MultiplayerLobby) {
					// マルチプレイヤーロビーへ
					currentMode = GameMode::MultiplayerLobby;
					multiplayerState = MultiplayerState::NotConnected;
				}
			}
			break;
		}

		case GameMode::MultiplayerLobby:
		{
			// Photonクライアントの初期化（まだ作成されていない場合）
			if (!photon) {
				photon.emplace(PHOTON_APP_ID, PHOTON_APP_VERSION);
			}

			// Photonの更新（必ず毎フレーム呼ぶ）
			photon->update();

			// 状態遷移の処理
			if (multiplayerState == MultiplayerState::Connecting) {
				// 接続処理（初回のみ）
				static bool connectCalled = false;
				if (!connectCalled) {
					photon->connect(U"Player" + ToString(Random(1000, 9999)));
					connectCalled = true;
					Print << U"[Debug] Connecting to Photon server...";
				}

				// 接続完了を待つ
				if (photon->isInLobby()) {
					Print << U"[Debug] Connected to lobby!";
					multiplayerState = MultiplayerState::InLobby;
					connectCalled = false; // リセット
				}
			}
			else if (multiplayerState == MultiplayerState::CreatingRoom)
			{
				// ルーム作成（初回のみ）
				static bool createCalled = false;
				if (!createCalled) {
					currentRoomCode = ToString(Random(1000, 9999));
					String roomName = U"BattleRoom_" + currentRoomCode;
					photon->createRoom(roomName, 2);
					createCalled = true;
					Print << U"[Debug] Creating room: " << roomName;
				}

				// ルーム参加完了を待つ
				if (photon->isInRoom()) {
					Print << U"[Debug] Room created! Waiting for opponent...";
					multiplayerState = MultiplayerState::InRoom;
					createCalled = false; // リセット
				}
			}
			else if (multiplayerState == MultiplayerState::JoiningRoom) {
				// ランダムルームに参加（初回のみ）
				static bool joinCalled = false;
				if (!joinCalled) {
					photon->joinRoom(U"BattleRoom_"+currentRoomCode);
					joinCalled = true;
					Print << U"[Debug] Joining random room...";
				}

				// ルーム参加完了を待つ
				if (photon->isInRoom()) {
					Print << U"[Debug] Joined room!";
					multiplayerState = MultiplayerState::InRoom;
					joinCalled = false; // リセット
				}
			}
			else if (multiplayerState == MultiplayerState::Ready) {
				// ゲーム開始準備完了 - ゲームを初期化
				object_pos = initializePos();
				gameState = initializeGame(object_pos, true); // マルチプレイモード

				// GameStateにネットワークを設定
				gameState->setNetwork(&(*photon));
				gameState->setMultiplayerMode(true, photon->isHost());
				photon->setGameState(gameState.get());

				// ホストの場合、ゲーム初期化イベントを送信
				if (photon->isHost()) {
					uint32_t seed = static_cast<uint32_t>(Time::GetMillisec());
					gameState->setGameSeed(seed);

					// ホスト側でもデッキをシードでシャッフル（初期手札を配る前に）
					gameState->getDeck()->shuffleWithSeed(seed);
					gameState->getSpecialDeck()->shuffleWithSeed(seed + 1);

					// 両プレイヤーに初期手札を配る（7枚ずつ）
					// 重要: デッキ同期の前に手札を配る
					for (int i = 0; i < 7; i++) {
						gameState->getPlayer1()->drawCard(gameState->getDeck());
						gameState->getPlayer2()->drawCard(gameState->getDeck());
					}

					Print << U"[Debug] Dealt initial hands";
					Print << U"[Debug] Player1 hand: " << gameState->getPlayer1()->getHand().size();
					Print << U"[Debug] Player2 hand: " << gameState->getPlayer2()->getHand().size();

					// 手札配布後のデッキの順序をシリアル化して送信（完全同期）
					s3d::Array<int32> deckOrder = gameState->getDeck()->serializeDeck();
					s3d::Array<int32> specialDeckOrder = gameState->getSpecialDeck()->serializeDeck();

					Print << U"[Debug] Sending deck sync: " << deckOrder.size() << U" normal cards, " << specialDeckOrder.size() << U" special cards";

					std::cout << "[Host] Special deck order being sent (first 10): ";
					for (int i = 0; i < specialDeckOrder.size() && i < 10; i++) {
						std::cout << specialDeckOrder[i] << " ";
					}
					std::cout << std::endl;

					// 重要: デッキ同期を送信（手札配布後のデッキ状態）
					gameState->sendDeckSyncEvent(deckOrder);
					gameState->sendSpecialDeckSyncEvent(specialDeckOrder);

					// デッキ同期が届くのを待つために少し待機
					System::Sleep(100ms);

					// GAME_INITイベントを送信
					gameState->sendGameInitEvent(seed);
				}

				// ゲームモードへ
				currentMode = GameMode::Playing;
			}

			// ロビー画面の描画と状態更新
			MultiplayerState newState = drawMultiplayerLobby(titleFont, buttonFont, infoFont,
				*photon, multiplayerState, roomNameState, currentRoomCode, gameState);

			// 「戻る」ボタンが押された場合
			if (newState == MultiplayerState::NotConnected && multiplayerState != MultiplayerState::NotConnected) {
				currentMode = GameMode::Menu;
				photon.reset(); // Photonクライアントをリセット
			}
			else {
				multiplayerState = newState;
			}

			break;
		}

		case GameMode::Playing:
		{
			// ゲームプレイ
			runGame(object_pos, gameState, photon ? &(*photon) : nullptr);

			// ゲーム終了後はメニューに戻る
			currentMode = GameMode::Menu;
			if (photon) {
				photon->disconnect();
				photon.reset();
			}
			break;
		}
		}
	}
}

