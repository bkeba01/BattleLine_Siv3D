#include <Siv3D.hpp>
#include <memory>
#include "core/Card.h"
#include "core/Deck.h"
#include "core/SpecialDeck.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/Flag.h"
#include "core/Slot.h"

std::unique_ptr<GameState> initializeGame(HashTable<String,Vec2> object_pos)
{
	RectF deck_card{ Arg::center((object_pos[U"card_slot_size"].x / 2) + 10,Scene::Height() / 2),object_pos[U"card_slot_size"].x,object_pos[U"card_slot_size"].y };
	Deck deck(Font{ 30, Typeface::Bold }, Texture{ U"🃏"_emoji },Texture{U"⚔"_emoji});
	deck.setRect(deck_card);
	deck.shuffle();

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
	specialDeck.shuffle();

	Player player1(0, deck, object_pos[U"card_hand_size"], object_pos[U"card_hand_space"]);
	Player player2(1, deck, object_pos[U"card_hand_size"], object_pos[U"card_opponent_hand_space"]);
	auto gameState = std::make_unique<GameState>(player1, player2, deck, specialDeck);
	gameState->setCurrentPlayer(gameState->getPlayer1());
	const Texture Flag_texture{ Image{ U"C:\\BattleLine\\BattleLine\\lib\\img\\ball_red.png" }.scaled(0.2) };

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
	Window::SetTitle(U"Battle Line");
	Window::Resize(1200, 900);
	

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

void Main()
{
	HashTable<String, Vec2> object_pos = initializePos();
	auto gameState = initializeGame(object_pos);

	// Initial update to set card positions
	gameState->getPlayer1()->update();
	gameState->getPlayer2()->update();
	Font instructionFont{ 24, Typeface::Bold };
	while (System::Update())
	{
		gameState->autoSetFinished();
		if (gameState->getFinished())
		{
			break;
		}

		gameState->getDeck()->drawDeck();
		gameState->getSpecialDeck()->drawDeck();

		// Get the current dragged card type to conditionally show empty slots
		DraggedCardType draggedType = gameState->getCurrentPlayer()->getDraggedCardType();

		// DeploymentCardモードでドラッグ中かチェック
		bool deploymentDragging = gameState->isDeploymentMode()
			&& gameState->getDeploymentSourceFlag() != -1
			&& MouseL.pressed();

		for (int flag = 0; flag < 9; flag++)
		{
			// Always draw normal Slot (placed cards), show empty slots only when dragging NormalCard or TroopCard
			// または DeploymentCard でドラッグ中で、勝敗が決まっていないFlagの場合
			bool isUndecidedFlag = gameState->getFlags()[flag].getFlagStatus() == ste_NonePlayer;
			bool showNormalSlots = (draggedType == DraggedCardType::NormalCard || draggedType == DraggedCardType::TroopCard)
				|| (deploymentDragging && isUndecidedFlag && flag != gameState->getDeploymentSourceFlag());
			gameState->getSlot(flag).slotdraw(*gameState, gameState->getCurrentPlayer()->getId(), showNormalSlots);

			// Always draw WeatherSlot (placed cards), show empty slots only when dragging WeatherCard
			bool showWeatherSlots = (draggedType == DraggedCardType::WeatherCard);
			gameState->getWeatherSlot(flag).slotdraw(*gameState, gameState->getCurrentPlayer()->getId(), showWeatherSlots);

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
		gameState->getConspiracySlot().slotdraw(*gameState, gameState->getCurrentPlayer()->getId(), showConspiracySlots);

		// Update visuals for both players
		gameState->getPlayer1()->update();
		gameState->getPlayer2()->update();

		// ReconCard (偵察カード) モードの処理
		if (gameState->isReconMode())
		{
			int phase = gameState->getReconPhase();
			auto& selectedCards = gameState->getReconDrawnCards();
			auto& cardFromSpecial = gameState->getReconCardFromSpecial();

			if (phase == 0)
			{
				// Phase 0: 山札選択（通常 or 特殊、複数回可能）
				instructionFont(U"山札を選択してカード一覧を表示（選択済: " + ToString(selectedCards.size()) + U"/3）").drawAt(Scene::Center().movedBy(0, -250), Palette::Orange);

				// 3枚選択済みなら次のフェーズへ進むボタンを表示
				if (selectedCards.size() == 3)
				{
					RectF nextButton(Scene::Center().x - 100, Scene::Center().y + 200, 200, 60);
					nextButton.draw(Palette::Green);
					instructionFont(U"次へ").drawAt(nextButton.center(), Palette::White);
					if (nextButton.leftClicked())
					{
						// 各山札をシャッフル
						gameState->getDeck()->shuffle();
						gameState->getSpecialDeck()->shuffle();
						// 手札に追加
						for (auto& card : selectedCards)
						{
							gameState->getCurrentPlayer()->addCardToHand(card);
						}
						gameState->setReconPhase(2); // Phase 2へ
					}
				}
				else
				{
					// デッキをホバー時にハイライト
					if (gameState->getDeck()->getRect().mouseOver())
					{
						gameState->getDeck()->getRect().drawFrame(5, Palette::Orange);
					}
					if (gameState->getSpecialDeck()->getRect().mouseOver())
					{
						gameState->getSpecialDeck()->getRect().drawFrame(5, Palette::Gold);
					}

					// 山札選択の入力処理
					if (gameState->getDeck()->getRect().leftClicked())
					{
						gameState->setReconViewingDeck(true);
						gameState->setReconViewingSpecial(false);
						gameState->setReconPhase(1);
					}
					else if (gameState->getSpecialDeck()->getRect().leftClicked())
					{
						gameState->setReconViewingDeck(false);
						gameState->setReconViewingSpecial(true);
						gameState->setReconPhase(1);
					}
				}
			}
			else if (phase == 1)
			{
				// Phase 1: 山札一覧から選択
				bool viewingSpecial = gameState->isReconViewingSpecial();
				String deckName = viewingSpecial ? U"特殊デッキ" : U"通常デッキ";
				instructionFont(deckName + U"一覧（選択済: " + ToString(selectedCards.size()) + U"/3）").drawAt(Scene::Center().movedBy(0, -350), Palette::Orange);

				// 山札の一覧を表示
				if (viewingSpecial)
				{
					const auto& cards = gameState->getSpecialDeck()->getCards();
					float cardWidth = 80;
					float cardHeight = 120;
					int cardsPerRow = 5;
					float startX = Scene::Center().x - (cardWidth * cardsPerRow + 10 * (cardsPerRow - 1)) / 2;
					float startY = Scene::Center().y - 150;

					for (size_t i = 0; i < cards.size(); ++i)
					{
						int row = i / cardsPerRow;
						int col = i % cardsPerRow;
						float cardX = startX + col * (cardWidth + 10);
						float cardY = startY + row * (cardHeight + 10);
						RectF cardRect(cardX, cardY, cardWidth, cardHeight);

						// カードを描画（位置指定版を使用）
						cards[i].draw(cardRect);

						// クリックで選択
						if (cardRect.leftClicked() && selectedCards.size() < 3)
						{
							auto removedCard = gameState->getSpecialDeck()->removeCard(i);
							if (removedCard)
							{
								selectedCards.push_back(removedCard);
								cardFromSpecial.push_back(true);
								gameState->setReconPhase(0); // Phase 0に戻る
							}
							break;
						}
					}
				}
				else
				{
					const auto& cards = gameState->getDeck()->getCards();
					float cardWidth = 80;
					float cardHeight = 120;
					int cardsPerRow = 10;
					float startX = Scene::Center().x - (cardWidth * cardsPerRow + 10 * (cardsPerRow - 1)) / 2;
					float startY = Scene::Center().y - 200;

					for (size_t i = 0; i < cards.size() && i < 30; ++i) // 最大30枚表示
					{
						int row = i / cardsPerRow;
						int col = i % cardsPerRow;
						float cardX = startX + col * (cardWidth + 10);
						float cardY = startY + row * (cardHeight + 10);
						RectF cardRect(cardX, cardY, cardWidth, cardHeight);

						// カードを描画（位置指定版を使用）
						cards[i].draw(cardRect);

						// クリックで選択
						if (cardRect.leftClicked() && selectedCards.size() < 3)
						{
							auto removedCard = gameState->getDeck()->removeCard(i);
							if (removedCard)
							{
								selectedCards.push_back(removedCard);
								cardFromSpecial.push_back(false);
								gameState->setReconPhase(0); // Phase 0に戻る
							}
							break;
						}
					}
				}

				// 戻るボタン
				RectF backButton(50, Scene::Height() - 100, 150, 50);
				backButton.draw(Palette::Gray);
				instructionFont(U"戻る").drawAt(backButton.center(), Palette::White);
				if (backButton.leftClicked())
				{
					gameState->setReconPhase(0);
				}
			}
			else if (phase == 2)
			{
				// Phase 2: 手札から2枚選択して、それぞれの山札に戻す
				auto& selectedIndices = gameState->getReconSelectedHandIndices();
				instructionFont(U"手札から2枚選択して山札に戻してください（選択: " + ToString(selectedIndices.size()) + U"/2）").drawAt(Scene::Center().movedBy(0, -250), Palette::Orange);

				// 手札のカードのクリック処理
				Player* currentPlayer = gameState->getCurrentPlayer();
				const auto& hand = currentPlayer->getHand();
				for (size_t i = 0; i < hand.size(); ++i)
				{
					if (hand[i] && hand[i]->getRect().leftClicked())
					{
						// 既に選択されているかチェック
						auto it = std::find(selectedIndices.begin(), selectedIndices.end(), static_cast<int>(i));
						if (it != selectedIndices.end())
						{
							// 選択解除
							selectedIndices.erase(it);
						}
						else if (selectedIndices.size() < 2)
						{
							// 選択追加（最大2枚）
							selectedIndices.push_back(static_cast<int>(i));
						}
						break;
					}
				}

				// 選択したカードをハイライト表示
				for (int index : selectedIndices)
				{
					if (index >= 0 && index < static_cast<int>(hand.size()) && hand[index])
					{
						hand[index]->getRect().drawFrame(5, Palette::Yellow);
					}
				}

				// 2枚選択したら確定ボタンを表示
				if (selectedIndices.size() == 2)
				{
					RectF confirmButton(Scene::Center().x - 100, Scene::Center().y + 200, 200, 60);
					confirmButton.draw(Palette::Green);
					instructionFont(U"確定").drawAt(confirmButton.center(), Palette::White);

					if (confirmButton.leftClicked())
					{
						// 選択したカードを山札に戻す
						// まず、選択されたカードを保存（インデックスでアクセスするため）
						std::vector<std::pair<int, std::shared_ptr<CardBase>>> cardsToReturn;
						for (int index : selectedIndices)
						{
							if (index >= 0 && index < static_cast<int>(hand.size()))
							{
								cardsToReturn.push_back({ index, hand[index] });
							}
						}

						// インデックスの大きい方から削除（配列の要素がずれないように）
						std::sort(selectedIndices.rbegin(), selectedIndices.rend());

						for (int index : selectedIndices)
						{
							gameState->getCurrentPlayer()->removeCardFromHandByIndex(index);
						}

						// カードをそれぞれの山札に戻す
						// 各カードが通常デッキ or 特殊デッキのどちらから来たかを判定
						for (const auto& [index, card] : cardsToReturn)
						{
							// CardかSpecialCardかで判定
							if (std::dynamic_pointer_cast<SpecialCard>(card))
							{
								gameState->getSpecialDeck()->returnCard(card);
							}
							else
							{
								gameState->getDeck()->returnCard(card);
							}
						}

						// 各山札をシャッフル
						gameState->getDeck()->shuffle();
						gameState->getSpecialDeck()->shuffle();

						// ReconModeを終了し、山札選択待ちへ
						gameState->setReconMode(false);
						gameState->setWaitingForDeckChoice(true);
					}
				}
			}
		}
		// DeploymentCard (配置展開カード) モードの処理
		else if (gameState->isDeploymentMode())
		{
			int sourceFlag = gameState->getDeploymentSourceFlag();
			int sourceSlot = gameState->getDeploymentSourceSlot();

			if (sourceFlag == -1)
			{
				// Phase 1: 自分のスロットカードを選択
				instructionFont(U"移動/削除するカードを選択してください").drawAt(Scene::Center().movedBy(0, -250), Palette::Cyan);

				// 勝敗が決まっていないFlagの自分のスロットカードをハイライト表示
				Player* currentPlayer = gameState->getCurrentPlayer();
				int playerId = currentPlayer->getId();

				for (int flagIndex = 0; flagIndex < 9; ++flagIndex)
				{
					Flag& flag = gameState->getFlags()[flagIndex];
					// 勝敗が決まっていないFlagのみ
					if (flag.getFlagStatus() == ste_NonePlayer)
					{
						Slot& slot = gameState->getSlot(flagIndex);
						// 自分のスロットカードをハイライト表示
						for (int slotIndex = 0; slotIndex <= static_cast<int>(ste_SlotCardMaxNum); ++slotIndex)
						{
							auto card = slot.getCard(playerId, slotIndex);
							if (card)
							{
								// ハイライト表示
								card->getRect().drawFrame(5, Palette::Cyan);

								// クリックされたら選択
								if (card->getRect().leftClicked())
								{
									gameState->setDeploymentSourceFlag(flagIndex);
									gameState->setDeploymentSourceSlot(slotIndex);
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				// Phase 2: 選択したカードをドラッグ＆ドロップ
				instructionFont(U"カードを別のSlotにドラッグ、または相手手札エリアにドロップして削除").drawAt(Scene::Center().movedBy(0, -250), Palette::Cyan);

				Player* currentPlayer = gameState->getCurrentPlayer();
				int playerId = currentPlayer->getId();
				Slot& sourceSlotObj = gameState->getSlot(sourceFlag);
				auto selectedCard = sourceSlotObj.getCard(playerId, sourceSlot);

				if (selectedCard)
				{
					// マウスを追従させてカードを表示（ドラッグ中）
					if (MouseL.pressed())
					{
						// 削除エリアを点線で表示
						float deleteAreaY = (playerId == 0) ? Scene::Height() * 0.1 : Scene::Height() * 0.9;
						RectF deleteArea(50, deleteAreaY - 50, Scene::Width() - 100, 100);
						deleteArea.draw(ColorF{ 1.0, 0.2, 0.2, 0.2 }); // 半透明の赤
						float thickness = 4.0;
						ColorF lineColor = Palette::Red;
						deleteArea.top().draw(LineStyle::SquareDot, thickness, lineColor);
						deleteArea.right().draw(LineStyle::SquareDot, thickness, lineColor);
						deleteArea.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
						deleteArea.left().draw(LineStyle::SquareDot, thickness, lineColor);
						instructionFont(U"削除エリア").drawAt(deleteArea.center(), Palette::Red);

						RectF draggedRect = selectedCard->getRect();
						draggedRect.setCenter(Cursor::Pos());
						selectedCard->setRect(draggedRect);
						selectedCard->draw();
						draggedRect.drawFrame(3, Palette::Yellow);
					}
					// マウスボタンが離されたらドロップ処理
					else if (MouseL.up())
					{
						Vec2 dropPos = Cursor::Pos();
						bool cardProcessed = false;

						// 削除エリア判定（ドラッグ中に表示していた矩形と同じ）
						float deleteAreaY = (playerId == 0) ? Scene::Height() * 0.1 : Scene::Height() * 0.9;
						RectF deleteArea(50, deleteAreaY - 50, Scene::Width() - 100, 100);
						if (deleteArea.contains(dropPos))
						{
							// カードを削除
							sourceSlotObj.getCards()[playerId][sourceSlot] = nullptr;
							cardProcessed = true;
						}
						else
						{
							// 別のスロットへの移動判定
							for (int targetFlagIndex = 0; targetFlagIndex < 9; ++targetFlagIndex)
							{
								Flag& targetFlag = gameState->getFlags()[targetFlagIndex];
								if (targetFlag.getFlagStatus() == ste_NonePlayer && targetFlagIndex != sourceFlag)
								{
									Slot& targetSlot = gameState->getSlot(targetFlagIndex);
									int emptySlot = targetSlot.checkCardSpace(currentPlayer, *gameState);
									if (emptySlot != static_cast<int>(ste_SlotCard_NonSpace))
									{
										RectF targetRect = targetSlot.getCardSlotRect(*gameState, playerId, emptySlot, playerId);
										if (targetRect.contains(dropPos))
										{
											// カードを移動
											targetSlot.getCards()[playerId][emptySlot] = selectedCard;
											sourceSlotObj.getCards()[playerId][sourceSlot] = nullptr;
											cardProcessed = true;
											break;
										}
									}
								}
							}
						}

						// 処理完了
						if (cardProcessed)
						{
							gameState->setDeploymentMode(false);
							gameState->setWaitingForDeckChoice(true);
						}
						else
						{
							// ドロップ失敗：選択を解除
							gameState->setDeploymentSourceFlag(-1);
							gameState->setDeploymentSourceSlot(-1);
						}
					}
				}
			}
		}
		// 山札選択待ちかどうかで処理を分岐
		else if (gameState->isWaitingForDeckChoice())
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


		// Define hand positions
		const Vec2 player_hand_pos = object_pos[U"card_hand_space"];
		const Vec2 opponent_hand_pos = { object_pos[U"card_hand_space"].x, Scene::Height() * 0.1 };

		// Identify players and set their hand positions for this frame
		Player* currentPlayer = gameState->getCurrentPlayer();
		Player* opponentPlayer = (currentPlayer == gameState->getPlayer1()) ? gameState->getPlayer2() : gameState->getPlayer1();

		currentPlayer->setHandSpace(player_hand_pos);
		opponentPlayer->setHandSpace(opponent_hand_pos);

		// Draw hands from the current player's perspective
		currentPlayer->draw(*gameState);
		opponentPlayer->drawBacks();
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
