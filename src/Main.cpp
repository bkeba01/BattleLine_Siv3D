#include <Siv3D.hpp>
#include "core/Card.h"
#include "core/Deck.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/Flag.h"

GameState initializeGame(HashTable<String,Vec2> object_pos)
{
	RectF deck_card{ Arg::center((object_pos[U"card_slot_size"].x / 2) + 10,Scene::Height() / 2),object_pos[U"card_slot_size"].x,object_pos[U"card_slot_size"].y };
	Deck deck(Font{ 30, Typeface::Bold }, Texture{ U"🃏"_emoji },Texture{U"⚔"_emoji});
	deck.setRect(deck_card);
	deck.shuffle();
	Player player1(0, deck, object_pos[U"card_hand_size"], object_pos[U"card_hand_space"]);
	Player player2(1, deck, object_pos[U"card_hand_size"], object_pos[U"card_opponent_hand_space"]);
	GameState gameState(player1, player2, deck);
	gameState.setCurrentPlayer(gameState.getPlayer1());
	const Texture Flag_texture{ Image{ U"C:\\BattleLine\\BattleLine\\lib\\img\\ball_red.png" }.scaled(0.2) };
	for (int flag = 0; flag < 9; flag++)
	{
		gameState.getFlags()[flag].setTexture(Flag_texture);
		gameState.getFlags()[flag].setCardSlotSize(object_pos[U"card_slot_size"]);
	}
	return gameState;
}
HashTable<String, Vec2> initializePos()
{
	Scene::SetBackground(ColorF{ 0.3, 0.6, 0.4 });
	Window::SetTitle(U"Battle Line");
	Window::Resize(1000, 900);

	const float card_hand_width = 120;
	const float card_hand_height = 180;
	const float card_slot_width = card_hand_width*0.7;
	const float card_slot_height = card_hand_height*0.7;
	const float card_hand_x_space = (Scene::Width() - (card_hand_width / 2 * 7)) / 2;
	const float player_hand_y = Scene::Height() * 0.9;
	const float opponent_hand_y = Scene::Height() * 0.1;
	const float flag_space_size_x = Scene::Width() - (card_slot_width);
	const float flag_y = Scene::Height() / 2;
	HashTable<String, Vec2>object_pos = {
		{U"card_hand_size",Vec2(card_hand_width,card_hand_height)},
		{U"card_slot_size",Vec2(card_slot_width,card_slot_height)},
		{U"card_hand_space",Vec2(card_hand_x_space,player_hand_y)},
		{U"flag_space_size",Vec2(flag_space_size_x,flag_y)}
	};
	return object_pos;
}

void Main()
{
	HashTable<String, Vec2> object_pos = initializePos();
	GameState gameState = initializeGame(object_pos);

	// Initial update to set card positions
	gameState.getPlayer1()->update();
	gameState.getPlayer2()->update();

	while (System::Update())
	{
		
		gameState.getDeck()->drawDeck();
		for (int flag = 0; flag < 9; flag++)
		{
			
			float flag_between_size = (object_pos[U"flag_space_size"].x - 9 * gameState.getFlags()[flag].getTexture().width())/10;
			float flag_x = object_pos[U"card_slot_size"].x + flag_between_size+(gameState.getFlags()[flag].getTexture().width()/2) +flag* (gameState.getFlags()[flag].getTexture().width()+ flag_between_size) ;
			
			Vec2 flag_position = { flag_x, object_pos[U"flag_space_size"].y};
			gameState.getFlags()[flag].setPos(flag_position);
			gameState.getFlags()[flag].draw();
			gameState.getFlags()[flag].slotdraw(gameState.getCurrentPlayer()->getId());
		}

		// Update visuals for both players
		gameState.getPlayer1()->update();
		gameState.getPlayer2()->update();

		// Handle input for the current player
		gameState.getCurrentPlayer()->handleInput(gameState);

		// Define hand positions
		const Vec2 player_hand_pos = object_pos[U"card_hand_space"];
		const Vec2 opponent_hand_pos = { object_pos[U"card_hand_space"].x, Scene::Height() * 0.1 };

		// Identify players and set their hand positions for this frame
		Player* currentPlayer = gameState.getCurrentPlayer();
		Player* opponentPlayer = (currentPlayer == gameState.getPlayer1()) ? gameState.getPlayer2() : gameState.getPlayer1();

		currentPlayer->setHandSpace(player_hand_pos);
		opponentPlayer->setHandSpace(opponent_hand_pos);

		// Draw hands from the current player's perspective
		currentPlayer->draw(gameState);
		opponentPlayer->drawBacks();
	}
}
