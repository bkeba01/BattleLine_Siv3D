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
	Font smallFont{ 16, Typeface::Bold };
	Font debugFont{ 14, Typeface::Bold };
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

				// 選択済みカードの表示（画面中央）
				if (!selectedCards.empty())
				{
					float cardWidth = object_pos[U"card_hand_size"].x;
					float cardHeight = object_pos[U"card_hand_size"].y;
					float cardSpacing = 20.0f;
					float totalWidth = selectedCards.size() * cardWidth + (selectedCards.size() - 1) * cardSpacing;
					float startX = (Scene::Width() - totalWidth) / 2.0f;
					float centerY = Scene::Center().y;

					smallFont(U"選択済みカード: (クリックで削除)").drawAt(Scene::Center().x, centerY - cardHeight / 2.0f - 30, Palette::Yellow);

					for (size_t i = 0; i < selectedCards.size(); ++i)
					{
						float cardX = startX + i * (cardWidth + cardSpacing);
						RectF cardRect(cardX, centerY - cardHeight / 2.0f, cardWidth, cardHeight);

						// カードを描画
						selectedCards[i]->draw(cardRect);

						// 黄色のフレームで選択済みを示す
						cardRect.drawFrame(5, Palette::Yellow);

						// クリックで削除
						if (cardRect.leftClicked())
						{
							// カードをデッキに戻す
							if (cardFromSpecial[i])
							{
								gameState->getSpecialDeck()->returnCard(selectedCards[i]);
							}
							else
							{
								gameState->getDeck()->returnCard(selectedCards[i]);
							}
							// リストから削除
							selectedCards.erase(selectedCards.begin() + i);
							cardFromSpecial.erase(cardFromSpecial.begin() + i);
							break;
						}
					}
				}

				// デッキをホバー時にハイライト（常に表示）
				if (gameState->getDeck()->getRect().mouseOver())
				{
					gameState->getDeck()->getRect().drawFrame(5, Palette::Orange);
				}
				if (gameState->getSpecialDeck()->getRect().mouseOver())
				{
					gameState->getSpecialDeck()->getRect().drawFrame(5, Palette::Gold);
				}

				// 山札選択の入力処理（常に可能）
				if (gameState->getDeck()->getRect().leftClicked())
				{
					gameState->setReconViewingDeck(true);
					gameState->setReconViewingSpecial(false);
					gameState->setReconScrollOffset(0.0); // スクロールをリセット
					gameState->setReconPhase(1);
				}
				else if (gameState->getSpecialDeck()->getRect().leftClicked())
				{
					gameState->setReconViewingDeck(false);
					gameState->setReconViewingSpecial(true);
					gameState->setReconScrollOffset(0.0); // スクロールをリセット
					gameState->setReconPhase(1);
				}

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
						// 手札に追加（force=trueで上限を無視）
						Vec2 handSpace = object_pos[U"card_hand_space"];
						if (gameState->getCurrentPlayer()->getId() == 1) {
							handSpace = object_pos[U"card_opponent_hand_space"];
						}
						for (auto& card : selectedCards)
						{
							// カードのサイズとスペースを設定
							card->setCardHandSize(object_pos[U"card_hand_size"]);
							card->setCardHandSpace(handSpace);
							gameState->getCurrentPlayer()->addCardToHand(card, true);
						}
						// 手札が更新されたので、updateを呼んで m_cardRects を更新
						gameState->getCurrentPlayer()->update();
						gameState->setReconPhase(2); // Phase 2へ
					}
				}
			}
			else if (phase == 1)
			{
				// Phase 1: 山札一覧から選択
				bool viewingSpecial = gameState->isReconViewingSpecial();
				String deckName = viewingSpecial ? U"特殊デッキ" : U"通常デッキ";

				// 選択中のカードをハイライトするためのインデックスリスト
				auto& selectedDeckIndices = gameState->getReconSelectedDeckIndices();

				int remaining_count = 3 - selectedCards.size();
				instructionFont(deckName + U"から" + ToString(remaining_count) + U"枚選択してください").drawAt(Scene::Center().movedBy(0, -350), Palette::Orange);

				// 選択状況の表示（デバッグ用）
				String statusText = U"選択済: " + ToString(selectedCards.size()) + U"枚 + 現在選択中: " + ToString(selectedDeckIndices.size()) + U"枚 = 合計 " + ToString(selectedCards.size() + selectedDeckIndices.size()) + U"/3枚";
				instructionFont(statusText).drawAt(Scene::Center().movedBy(0, -320), Palette::Yellow);

				// ホバー管理
				HoverManager hoverManager;
				Array<RectF> cardRects;

				// カードのレイアウト設定と Rect の事前計算
				// スロットサイズでカードを表示（手札形式：カードを半分ずつ重ねて表示）

				// カードサイズ（スロットと同じ）
				float cardWidth = object_pos[U"card_slot_size"].x;
				float cardHeight = object_pos[U"card_slot_size"].y;

				// カード枚数を取得
				size_t cardCount = viewingSpecial ?
					gameState->getSpecialDeck()->getCards().size() :
					gameState->getDeck()->getCards().size();

				// 手札形式の総幅を計算（各カードが半分ずつ重なる）
				float cardSpacing = cardWidth / 2.0f;
				float totalWidth = cardSpacing * (cardCount - 1) + cardWidth;

				// 利用可能な画面幅（両端にマージンを確保）
				const float screenMargin = 100.0f;
				const float availableWidth = Scene::Width() - screenMargin * 2;

				// スクロール可能かどうか
				bool needsScroll = totalWidth > availableWidth;

				// スクロールオフセットの取得と更新
				double scrollOffset = gameState->getReconScrollOffset();

				// Y座標（中央）
				float centerY = Scene::Center().y;

				// カード表示領域の定義
				RectF viewArea(screenMargin, centerY - cardHeight / 2.0f - 50, availableWidth, cardHeight + 100);

				if (needsScroll)
				{
					// スクロール処理
					// マウスホイールでスクロール
					scrollOffset += Mouse::Wheel() * 30.0;

					// 左右矢印キーでもスクロール
					if (KeyLeft.pressed())
					{
						scrollOffset += 5.0;
					}
					if (KeyRight.pressed())
					{
						scrollOffset -= 5.0;
					}

					// スクロール範囲の制限
					double maxScroll = 0.0;
					double minScroll = -(totalWidth - availableWidth);
					scrollOffset = Clamp(scrollOffset, minScroll, maxScroll);

					// スクロールバーの表示
					float barWidth = availableWidth * (availableWidth / totalWidth);
					float barX = screenMargin - (scrollOffset / minScroll) * (availableWidth - barWidth);
					RectF scrollBar(barX, viewArea.y + viewArea.h - 10, barWidth, 8);
					scrollBar.draw(ColorF(0.5, 0.5, 0.5, 0.7));

					// スクロール可能であることを示すテキスト
					smallFont(U"← → キーまたはマウスホイールでスクロール").drawAt(Scene::Center().x, viewArea.y + viewArea.h + 20, Palette::Gray);
				}
				else
				{
					// スクロール不要な場合はリセット
					scrollOffset = 0.0;
				}

				// スクロールオフセットを保存（次のフレームで使用するため）
				gameState->setReconScrollOffset(scrollOffset);

				// カードのRectを計算（スクロールオフセットを適用）
				float startX = needsScroll ? screenMargin + scrollOffset : (Scene::Width() - totalWidth) / 2.0f;

				for (size_t i = 0; i < cardCount; ++i)
				{
					const double centerX = startX + cardWidth / 2.0f + i * cardSpacing;
					cardRects << RectF{ Arg::center(centerX, centerY), cardWidth, cardHeight };
				}

				hoverManager.updateHover(cardRects);
				const auto& hoveredIndex = hoverManager.hoveredIndex();

				// カードの描画（ホバーされていないもの）
				if (viewingSpecial)
				{
					const auto& cards = gameState->getSpecialDeck()->getCards();
					for (size_t i = 0; i < cards.size(); ++i)
					{
						if (hoveredIndex && *hoveredIndex == i) continue;
						// 表示領域内のみ描画
						if (cardRects[i].x + cardRects[i].w > screenMargin && cardRects[i].x < Scene::Width() - screenMargin)
						{
							cards[i].draw(cardRects[i]);
						}
					}
				}
				else
				{
					const auto& cards = gameState->getDeck()->getCards();
					for (size_t i = 0; i < cards.size(); ++i)
					{
						if (hoveredIndex && *hoveredIndex == i) continue;
						// 表示領域内のみ描画
						if (cardRects[i].x + cardRects[i].w > screenMargin && cardRects[i].x < Scene::Width() - screenMargin)
						{
							cards[i].draw(cardRects[i]);
						}
					}
				}

				// 選択されているカードのハイライト表示（ホバーされていないもの）
				for (size_t i = 0; i < cardRects.size(); ++i)
				{
					// ホバーされているカードはスキップ（後で拡大表示時に描画）
					if (hoveredIndex && *hoveredIndex == i)
						continue;

					// 表示領域外はスキップ
					if (cardRects[i].x + cardRects[i].w <= screenMargin || cardRects[i].x >= Scene::Width() - screenMargin)
						continue;

					auto it = std::find(selectedDeckIndices.begin(), selectedDeckIndices.end(), i);
					if (it != selectedDeckIndices.end())
					{
						cardRects[i].drawFrame(5, Palette::Yellow);
					}
				}

				// ホバーされているカードを最後に描画
				RectF enlargedCard; // ホバーされているカードの拡大Rect
				bool hasEnlargedCard = false;

				if (hoveredIndex)
				{
					const int i = *hoveredIndex;
					// 表示領域内のみ描画
					if (cardRects[i].x + cardRects[i].w > screenMargin && cardRects[i].x < Scene::Width() - screenMargin)
					{
						enlargedCard = cardRects[i].scaledAt(cardRects[i].center(), 1.15).moveBy(0, -20);
						hasEnlargedCard = true;

						if (viewingSpecial)
						{
							gameState->getSpecialDeck()->getCards()[i].draw(enlargedCard);
						}
						else
						{
							gameState->getDeck()->getCards()[i].draw(enlargedCard);
						}

						// ホバーされているカードが選択されている場合、拡大されたカードにフレームを描画
						auto it = std::find(selectedDeckIndices.begin(), selectedDeckIndices.end(), static_cast<size_t>(i));
						if (it != selectedDeckIndices.end())
						{
							enlargedCard.drawFrame(5, Palette::Yellow);
						}
					}
				}

				// クリック処理（ホバー中の拡大カードを優先、なければ最前面のカードを選択）
				if (MouseL.down())
				{
					bool cardClicked = false;

					// まずホバー中の拡大カードをチェック
					if (hasEnlargedCard && hoveredIndex)
					{
						if (enlargedCard.mouseOver())
						{
							const int i = *hoveredIndex;
							auto it = std::find(selectedDeckIndices.begin(), selectedDeckIndices.end(), static_cast<size_t>(i));
							if (it != selectedDeckIndices.end())
							{
								selectedDeckIndices.erase(it);
							}
							else if ((selectedDeckIndices.size() + selectedCards.size()) < 3)
							{
								selectedDeckIndices.push_back(static_cast<size_t>(i));
							}
							cardClicked = true;
						}
					}

					// ホバーカードがクリックされなかった場合、通常のクリック判定
					if (!cardClicked)
					{
						// 最前面のカード（右側のカード）から逆順にチェック
						for (int i = static_cast<int>(cardRects.size()) - 1; i >= 0; --i)
						{
							// 表示領域外はスキップ
							if (cardRects[i].x + cardRects[i].w <= screenMargin || cardRects[i].x >= Scene::Width() - screenMargin)
								continue;

							if (cardRects[i].mouseOver())
							{
								auto it = std::find(selectedDeckIndices.begin(), selectedDeckIndices.end(), static_cast<size_t>(i));
								if (it != selectedDeckIndices.end())
								{
									selectedDeckIndices.erase(it);
								}
								else if ((selectedDeckIndices.size() + selectedCards.size()) < 3)
								{
									selectedDeckIndices.push_back(static_cast<size_t>(i));
								}
								break; // 最初に当たったカードのみ処理
							}
						}
					}
				}

				// 合計3枚選択したら決定ボタンを表示
				size_t totalSelected = selectedDeckIndices.size() + selectedCards.size();

				// デバッグ: 決定ボタン表示条件の確認
				debugFont(U"totalSelected=" + ToString(totalSelected) + U", 決定ボタン: " + (totalSelected == 3 ? U"表示" : U"非表示")).draw(10, 10, Palette::White);

				// 確定ボタン（選択したカードがある場合のみ表示）
				if (!selectedDeckIndices.empty())
				{
					RectF confirmButton(Scene::Center().x - 100, Scene::Height() - 150, 200, 60);
					confirmButton.draw(Palette::Green);
					instructionFont(U"確定").drawAt(confirmButton.center(), Palette::White);
					if (confirmButton.leftClicked())
					{
						// 選択したカードをインデックスの大きい順に処理
						std::sort(selectedDeckIndices.rbegin(), selectedDeckIndices.rend());

						for (size_t index : selectedDeckIndices)
						{
							if (viewingSpecial)
							{
								auto removedCard = gameState->getSpecialDeck()->removeCard(index);
								if (removedCard)
								{
									selectedCards.push_back(removedCard);
									cardFromSpecial.push_back(true);
								}
							}
							else
							{
								auto removedCard = gameState->getDeck()->removeCard(index);
								if (removedCard)
								{
									selectedCards.push_back(removedCard);
									cardFromSpecial.push_back(false);
								}
							}
						}
						selectedDeckIndices.clear();
						gameState->setReconPhase(0); // Phase 0に戻る
					}
				}

				// 戻るボタン
				RectF backButton(50, Scene::Height() - 100, 150, 50);
				backButton.draw(Palette::Gray);
				instructionFont(U"戻る").drawAt(backButton.center(), Palette::White);
				if (backButton.leftClicked())
				{
					// 選択中のカードがあれば保存してから戻る
					if (!selectedDeckIndices.empty())
					{
						// 選択したカードをインデックスの大きい順に処理
						std::sort(selectedDeckIndices.rbegin(), selectedDeckIndices.rend());

						for (size_t index : selectedDeckIndices)
						{
							if (viewingSpecial)
							{
								auto removedCard = gameState->getSpecialDeck()->removeCard(index);
								if (removedCard)
								{
									selectedCards.push_back(removedCard);
									cardFromSpecial.push_back(true);
								}
							}
							else
							{
								auto removedCard = gameState->getDeck()->removeCard(index);
								if (removedCard)
								{
									selectedCards.push_back(removedCard);
									cardFromSpecial.push_back(false);
								}
							}
						}
						selectedDeckIndices.clear();
					}
					gameState->setReconPhase(0);
				}
			}
			else if (phase == 2)
			{
				// Phase 2: 手札から2枚選択して、それぞれの山札に戻す
				auto& selectedIndices = gameState->getReconSelectedHandIndices();

				// 念のため、Phase 2 開始時に選択済みカード情報をクリア（Phase 0から持ち越さない）
				selectedCards.clear();
				cardFromSpecial.clear();

				instructionFont(U"手札から2枚選択して山札に戻してください（選択: " + ToString(selectedIndices.size()) + U"/2）").drawAt(Scene::Center().movedBy(0, -250), Palette::Orange);

				// 手札のカードのクリック処理
				Player* currentPlayer = gameState->getCurrentPlayer();
				const auto& hand = currentPlayer->getHand();

				// DragManagerを更新してホバー判定を有効にする
				Array<RectF> handCardRects;
				for (int i = 0; i < hand.size(); ++i)
				{
					if (hand[i])
					{
						handCardRects << hand[i]->getRect();
					}
				}
				currentPlayer->updateDrag(handCardRects);

				// クリック処理
				if (MouseL.down())
				{
					bool cardClicked = false;

					// まずホバー中のカードをチェック
					const auto& hoveredIndex = currentPlayer->getDragManager().hoveredIndex();
					if (hoveredIndex)
					{
						int i = *hoveredIndex;
						auto it = std::find(selectedIndices.begin(), selectedIndices.end(), i);
						if (it != selectedIndices.end())
						{
							// 選択解除
							selectedIndices.erase(it);
						}
						else if (selectedIndices.size() < 2)
						{
							// 選択追加（最大2枚）
							selectedIndices.push_back(i);
						}
						cardClicked = true;
					}

					// ホバー中でない場合は、最前面のカード（右側のカード）から逆順にチェック
					if (!cardClicked)
					{
						for (int i = static_cast<int>(hand.size()) - 1; i >= 0; --i)
						{
							if (hand[i] && hand[i]->getRect().mouseOver())
							{
								auto it = std::find(selectedIndices.begin(), selectedIndices.end(), i);
								if (it != selectedIndices.end())
								{
									// 選択解除
									selectedIndices.erase(it);
								}
								else if (selectedIndices.size() < 2)
								{
									// 選択追加（最大2枚）
									selectedIndices.push_back(i);
								}
								break;
							}
						}
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
						std::vector<std::shared_ptr<CardBase>> cardsToReturn;
						for (int index : selectedIndices)
						{
							if (index >= 0 && index < static_cast<int>(hand.size()))
							{
								cardsToReturn.push_back(hand[index]);
							}
						}

						// インデックスの大きい方から手札から削除
						std::sort(selectedIndices.rbegin(), selectedIndices.rend());
						for (int index : selectedIndices)
						{
							gameState->getCurrentPlayer()->removeCardFromHandByIndex(index);
						}

						// カードをそれぞれの山札の "上" に戻す
						for (const auto& card : cardsToReturn)
						{
							if (std::dynamic_pointer_cast<SpecialCard>(card))
							{
								gameState->getSpecialDeck()->returnCard(card);
							}
							else
							{
								gameState->getDeck()->returnCard(card);
							}
						}
						// 選択インデックスをクリア
						selectedIndices.clear();
						// 選択したカード情報もクリア
						selectedCards.clear();
						cardFromSpecial.clear();

						// ReconModeを終了し、山札選択をスキップして次のプレイヤーに交代
						gameState->setReconMode(false);
						gameState->setWaitingForDeckChoice(false);
						gameState->changePlayer();
					}
				}
			}
		}
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

		int sourceFlag = gameState->getDeploymentSourceFlag();
		int sourceSlot = gameState->getDeploymentSourceSlot();
		if (gameState->isDeploymentMode())
		{
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

		// ReconModeのPhase 2では選択したカードに黄色のフレームを描画
		if (gameState->isReconMode() && gameState->getReconPhase() == 2)
		{
			auto& selectedIndices = gameState->getReconSelectedHandIndices();
			const auto& hand = currentPlayer->getHand();
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
