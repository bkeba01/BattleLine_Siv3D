// GameStateの特殊カード処理メソッドの実装
// Main.cppから分離してコードを整理

#include "core/GameState.h"
#include "core/Slot.h"
#include "core/Flag.h"
#include "core/Player.h"
#include "core/SpecialCard.h"
#include "core/HoverManager.h"

using namespace s3d;

// ReconCard（偵察カード）の処理
void GameState::handleReconCard(const Font& instructionFont, const Font& smallFont, const HashTable<String, Vec2>& object_pos)
{
	if (!isReconMode()) return;

	int phase = getReconPhase();
	auto& selectedCards = getReconDrawnCards();
	auto& cardFromSpecial = getReconCardFromSpecial();

	if (phase == 0)
	{
		// Phase 0: 山札選択（通常 or 特殊、複数回可能）
		instructionFont(U"山札を選択してカード一覧を表示（選択済: " + ToString(selectedCards.size()) + U"/3）").drawAt(Scene::Center().movedBy(0, -250), Palette::Orange);

		// 選択済みカードの表示（画面中央）
		if (!selectedCards.empty())
		{
			float cardWidth = object_pos.at(U"card_hand_size").x;
			float cardHeight = object_pos.at(U"card_hand_size").y;
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
						getSpecialDeck()->returnCard(selectedCards[i]);
					}
					else
					{
						getDeck()->returnCard(selectedCards[i]);
					}
					// リストから削除
					selectedCards.erase(selectedCards.begin() + i);
					cardFromSpecial.erase(cardFromSpecial.begin() + i);
					break;
				}
			}
		}

		// デッキをホバー時にハイライト（常に表示）
		if (getDeck()->getRect().mouseOver())
		{
			getDeck()->getRect().drawFrame(5, Palette::Orange);
		}
		if (getSpecialDeck()->getRect().mouseOver())
		{
			getSpecialDeck()->getRect().drawFrame(5, Palette::Gold);
		}

		// 山札選択の入力処理（常に可能）
		if (getDeck()->getRect().leftClicked())
		{
			setReconViewingDeck(true);
			setReconViewingSpecial(false);
			setReconScrollOffset(0.0);
			setReconPhase(1);
		}
		else if (getSpecialDeck()->getRect().leftClicked())
		{
			setReconViewingDeck(false);
			setReconViewingSpecial(true);
			setReconScrollOffset(0.0);
			setReconPhase(1);
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
				getDeck()->shuffle();
				getSpecialDeck()->shuffle();
				// 手札に追加（force=trueで上限を無視）
				Vec2 handSpace = object_pos.at(U"card_hand_space");
				if (getCurrentPlayer()->getId() == 1) {
					handSpace = object_pos.at(U"card_opponent_hand_space");
				}
				for (auto& card : selectedCards)
				{
					// カードのサイズとスペースを設定
					card->setCardHandSize(object_pos.at(U"card_hand_size"));
					card->setCardHandSpace(handSpace);
					getCurrentPlayer()->addCardToHand(card, true);
				}
				// 手札が更新されたので、updateを呼んで m_cardRects を更新
				getCurrentPlayer()->update();
				setReconPhase(2); // Phase 2へ
			}
		}
	}
	else if (phase == 1)
	{
		// Phase 1: 山札一覧から選択
		bool viewingSpecial = isReconViewingSpecial();
		String deckName = viewingSpecial ? U"特殊デッキ" : U"通常デッキ";

		// 選択中のカードをハイライトするためのインデックスリスト
		auto& selectedDeckIndices = getReconSelectedDeckIndices();

		int remaining_count = 3 - selectedCards.size();
		instructionFont(deckName + U"から" + ToString(remaining_count) + U"枚選択してください").drawAt(Scene::Center().movedBy(0, -350), Palette::Orange);

		// 選択状況の表示
		String statusText = U"選択済: " + ToString(selectedCards.size()) + U"枚 + 現在選択中: " + ToString(selectedDeckIndices.size()) + U"枚 = 合計 " + ToString(selectedCards.size() + selectedDeckIndices.size()) + U"/3枚";
		instructionFont(statusText).drawAt(Scene::Center().movedBy(0, -320), Palette::Yellow);

		// ホバー管理
		HoverManager hoverManager;
		Array<RectF> cardRects;

		// カードサイズ（スロットと同じ）
		float cardWidth = object_pos.at(U"card_slot_size").x;
		float cardHeight = object_pos.at(U"card_slot_size").y;

		// カード枚数を取得
		size_t cardCount = viewingSpecial ?
			getSpecialDeck()->getCards().size() :
			getDeck()->getCards().size();

		// 手札形式の総幅を計算（各カードが半分ずつ重なる）
		float cardSpacing = cardWidth / 2.0f;
		float totalWidth = cardSpacing * (cardCount - 1) + cardWidth;

		// 利用可能な画面幅（両端にマージンを確保）
		const float screenMargin = 100.0f;
		const float availableWidth = Scene::Width() - screenMargin * 2;

		// スクロール可能かどうか
		bool needsScroll = totalWidth > availableWidth;

		// スクロールオフセットの取得と更新
		double scrollOffset = getReconScrollOffset();

		// Y座標（中央）
		float centerY = Scene::Center().y;

		// カード表示領域の定義
		RectF viewArea(screenMargin, centerY - cardHeight / 2.0f - 50, availableWidth, cardHeight + 100);

		if (needsScroll)
		{
			// スクロール処理
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

		// スクロールオフセットを保存
		setReconScrollOffset(scrollOffset);

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
			const auto& cards = getSpecialDeck()->getCards();
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
			const auto& cards = getDeck()->getCards();
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
		RectF enlargedCard;
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
					getSpecialDeck()->getCards()[i].draw(enlargedCard);
				}
				else
				{
					getDeck()->getCards()[i].draw(enlargedCard);
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
						break;
					}
				}
			}
		}

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
						auto removedCard = getSpecialDeck()->removeCard(index);
						if (removedCard)
						{
							selectedCards.push_back(removedCard);
							cardFromSpecial.push_back(true);
						}
					}
					else
					{
						auto removedCard = getDeck()->removeCard(index);
						if (removedCard)
						{
							selectedCards.push_back(removedCard);
							cardFromSpecial.push_back(false);
						}
					}
				}
				selectedDeckIndices.clear();
				setReconPhase(0); // Phase 0に戻る
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
						auto removedCard = getSpecialDeck()->removeCard(index);
						if (removedCard)
						{
							selectedCards.push_back(removedCard);
							cardFromSpecial.push_back(true);
						}
					}
					else
					{
						auto removedCard = getDeck()->removeCard(index);
						if (removedCard)
						{
							selectedCards.push_back(removedCard);
							cardFromSpecial.push_back(false);
						}
					}
				}
				selectedDeckIndices.clear();
			}
			setReconPhase(0);
		}
	}
	else if (phase == 2)
	{
		// Phase 2: 手札から2枚選択して、それぞれの山札に戻す
		auto& selectedIndices = getReconSelectedHandIndices();

		// 念のため、Phase 2 開始時に選択済みカード情報をクリア
		selectedCards.clear();
		cardFromSpecial.clear();

		instructionFont(U"手札から2枚選択して山札に戻してください（選択: " + ToString(selectedIndices.size()) + U"/2）").drawAt(Scene::Center().movedBy(0, -250), Palette::Orange);

		// 手札のカードのクリック処理
		Player* currentPlayer = getCurrentPlayer();
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
					getCurrentPlayer()->removeCardFromHandByIndex(index);
				}

				// カードをそれぞれの山札の "上" に戻す
				for (const auto& card : cardsToReturn)
				{
					if (std::dynamic_pointer_cast<SpecialCard>(card))
					{
						getSpecialDeck()->returnCard(card);
					}
					else
					{
						getDeck()->returnCard(card);
					}
				}
				// 選択インデックスをクリア
				selectedIndices.clear();
				// 選択したカード情報もクリア
				selectedCards.clear();
				cardFromSpecial.clear();

				// ReconModeを終了し、山札選択をスキップして次のプレイヤーに交代
				setReconMode(false);
				setWaitingForDeckChoice(false);
				changePlayer();
			}
		}
	}
}

// DeploymentCard（配置展開カード）の処理
void GameState::handleDeploymentCard(const Font& instructionFont)
{
	if (!isDeploymentMode()) return;

	int sourceFlag = getDeploymentSourceFlag();
	int sourceSlot = getDeploymentSourceSlot();

	if (sourceFlag == -1)
	{
		// Phase 1: 自分のスロットカードを選択
		instructionFont(U"移動/削除するカードを選択してください").drawAt(Scene::Center().movedBy(0, -250), Palette::Cyan);

		// 勝敗が決まっていないFlagの自分のスロットカードをハイライト表示
		Player* currentPlayer = getCurrentPlayer();
		int playerId = currentPlayer->getId();

		for (int flagIndex = 0; flagIndex < 9; ++flagIndex)
		{
			Flag& flag = getFlags()[flagIndex];
			// 勝敗が決まっていないFlagのみ
			if (flag.getFlagStatus() == ste_NonePlayer)
			{
				Slot& slot = getSlot(flagIndex);
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
							setDeploymentSourceFlag(flagIndex);
							setDeploymentSourceSlot(slotIndex);
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

		Player* currentPlayer = getCurrentPlayer();
		int playerId = currentPlayer->getId();
		Slot& sourceSlotObj = getSlot(sourceFlag);
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
						Flag& targetFlag = getFlags()[targetFlagIndex];
						if (targetFlag.getFlagStatus() == ste_NonePlayer && targetFlagIndex != sourceFlag)
						{
							Slot& targetSlot = getSlot(targetFlagIndex);
							int emptySlot = targetSlot.checkCardSpace(currentPlayer, *this);
							if (emptySlot != static_cast<int>(ste_SlotCard_NonSpace))
							{
								RectF targetRect = targetSlot.getCardSlotRect(*this, playerId, emptySlot, playerId);
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
					setDeploymentMode(false);
					setWaitingForDeckChoice(true);
				}
				else
				{
					// ドロップ失敗：選択を解除
					setDeploymentSourceFlag(-1);
					setDeploymentSourceSlot(-1);
				}
			}
		}
	}
}

// EscapeCard（脱走カード）の処理
void GameState::handleEscapeCard(const Font& instructionFont)
{
	if (!isEscapeMode()) return;

	int targetFlag = getEscapeTargetFlag();
	int targetSlot = getEscapeTargetSlot();

	if (targetFlag == -1)
	{
		// Phase 1: 相手のスロットカードを選択（statusがnoneのフラグのみ）
		instructionFont(U"削除する相手のカードを選択してください").drawAt(Scene::Center().movedBy(0, -250), Palette::Red);

		// 現在のプレイヤーと相手プレイヤーを取得
		Player* opponentPlayer = getOpponentPlayer();
		int opponentId = opponentPlayer->getId();

		// 勝敗が決まっていないFlagの相手のスロットカードをハイライト表示
		for (int flagIndex = 0; flagIndex < 9; ++flagIndex)
		{
			Flag& flag = getFlags()[flagIndex];
			// 勝敗が決まっていないFlagのみ
			if (flag.getFlagStatus() == ste_NonePlayer)
			{
				Slot& slot = getSlot(flagIndex);
				// 相手のスロットカードをハイライト表示
				for (int slotIndex = 0; slotIndex <= static_cast<int>(ste_SlotCardMaxNum); ++slotIndex)
				{
					auto card = slot.getCard(opponentId, slotIndex);
					if (card)
					{
						// ハイライト表示（赤色）
						card->getRect().drawFrame(5, Palette::Red);

						// クリックされたら選択
						if (card->getRect().leftClicked())
						{
							setEscapeTargetFlag(flagIndex);
							setEscapeTargetSlot(slotIndex);
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		// Phase 2: 選択したカードの削除確認
		instructionFont(U"選択したカードを削除します（確認ボタンをクリック）").drawAt(Scene::Center().movedBy(0, -250), Palette::Red);

		Player* opponentPlayer = getOpponentPlayer();
		int opponentId = opponentPlayer->getId();
		Slot& targetSlotObj = getSlot(targetFlag);
		auto selectedCard = targetSlotObj.getCard(opponentId, targetSlot);

		if (selectedCard)
		{
			// 選択されたカードをハイライト表示
			selectedCard->getRect().drawFrame(8, Palette::Yellow);

			// 確認ボタンを表示
			RectF confirmButton(Scene::Center().x - 100, Scene::Center().y + 150, 200, 60);
			confirmButton.draw(Palette::Red);
			instructionFont(U"削除").drawAt(confirmButton.center(), Palette::White);
			if (confirmButton.leftClicked())
			{
				// カードを削除
				targetSlotObj.getCards()[opponentId][targetSlot] = nullptr;
				// モードを終了
				setEscapeMode(false);
				setWaitingForDeckChoice(true);
			}

			// キャンセルボタンを表示
			RectF cancelButton(Scene::Center().x - 100, Scene::Center().y + 230, 200, 60);
			cancelButton.draw(Palette::Gray);
			instructionFont(U"キャンセル").drawAt(cancelButton.center(), Palette::White);
			if (cancelButton.leftClicked())
			{
				// 選択を解除
				setEscapeTargetFlag(-1);
				setEscapeTargetSlot(-1);
			}
		}
	}
}

// BetrayalCard（裏切りカード）の処理
void GameState::handleBetrayalCard(const Font& instructionFont)
{
	if (!isBetrayalMode()) return;

	int betrayalSourceFlag = getBetrayalSourceFlag();
	int betrayalSourceSlot = getBetrayalSourceSlot();

	if (betrayalSourceFlag == -1)
	{
		// Phase 1: 相手のスロットカードを選択（statusがnoneのフラグのみ）
		instructionFont(U"裏切らせる相手のカードを選択してください").drawAt(Scene::Center().movedBy(0, -250), Palette::Purple);

		// 現在のプレイヤーと相手プレイヤーを取得
		Player* opponentPlayer = getOpponentPlayer();
		int opponentId = opponentPlayer->getId();

		// 勝敗が決まっていないFlagの相手のスロットカードをハイライト表示
		for (int flagIndex = 0; flagIndex < 9; ++flagIndex)
		{
			Flag& flag = getFlags()[flagIndex];
			// 勝敗が決まっていないFlagのみ
			if (flag.getFlagStatus() == ste_NonePlayer)
			{
				Slot& slot = getSlot(flagIndex);
				// 相手のスロットカードをハイライト表示
				for (int slotIndex = 0; slotIndex <= static_cast<int>(ste_SlotCardMaxNum); ++slotIndex)
				{
					auto card = slot.getCard(opponentId, slotIndex);
					if (card)
					{
						// ハイライト表示（紫色）
						card->getRect().drawFrame(5, Palette::Purple);

						// クリックされたら選択
						if (card->getRect().leftClicked())
						{
							setBetrayalSourceFlag(flagIndex);
							setBetrayalSourceSlot(slotIndex);
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
		instructionFont(U"カードを自分の空いているSlotにドロップして裏切らせる").drawAt(Scene::Center().movedBy(0, -250), Palette::Purple);

		Player* currentPlayer = getCurrentPlayer();
		Player* opponentPlayer = getOpponentPlayer();
		int currentPlayerId = currentPlayer->getId();
		int opponentId = opponentPlayer->getId();
		Slot& sourceSlotObj = getSlot(betrayalSourceFlag);
		auto selectedCard = sourceSlotObj.getCard(opponentId, betrayalSourceSlot);

		if (selectedCard)
		{
			// マウスを追従させてカードを表示（ドラッグ中）
			if (MouseL.pressed())
			{
				RectF draggedRect = selectedCard->getRect();
				draggedRect.setCenter(Cursor::Pos());
				selectedCard->setRect(draggedRect);
				selectedCard->draw();
				draggedRect.drawFrame(3, Palette::Purple);
			}
			// マウスボタンが離されたらドロップ処理
			else if (MouseL.up())
			{
				Vec2 dropPos = Cursor::Pos();
				bool cardProcessed = false;

				// 自分の空いているスロットへの移動判定
				for (int targetFlagIndex = 0; targetFlagIndex < 9; ++targetFlagIndex)
				{
					Flag& targetFlag = getFlags()[targetFlagIndex];
					if (targetFlag.getFlagStatus() == ste_NonePlayer)
					{
						Slot& targetSlot = getSlot(targetFlagIndex);
						int emptySlot = targetSlot.checkCardSpace(currentPlayer, *this);
						if (emptySlot != static_cast<int>(ste_SlotCard_NonSpace))
						{
							RectF targetRect = targetSlot.getCardSlotRect(*this, currentPlayerId, emptySlot, currentPlayerId);
							if (targetRect.contains(dropPos))
							{
								// カードを移動
								targetSlot.getCards()[currentPlayerId][emptySlot] = selectedCard;
								sourceSlotObj.getCards()[opponentId][betrayalSourceSlot] = nullptr;
								cardProcessed = true;
								break;
							}
						}
					}
				}

				// 処理完了
				if (cardProcessed)
				{
					setBetrayalMode(false);
					setWaitingForDeckChoice(true);
				}
				else
				{
					// ドロップ失敗：選択を解除
					setBetrayalSourceFlag(-1);
					setBetrayalSourceSlot(-1);
				}
			}
		}
	}
}
