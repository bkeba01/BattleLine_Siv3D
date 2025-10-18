#include <Siv3D.hpp>
#include "core/Card.h"
#include "core/Deck.h"
#include "core/Player.h"
#include "core/GameState.h"
#include "core/Flag.h"

GameState initializeGame(HashTable<String,Vec2> object_pos)
{
	Deck deck(Font{ 30, Typeface::Bold }, Texture{ U"🃏"_emoji });
	deck.shuffle();
	Player player1(1, deck, object_pos[U"card_hand_size"], object_pos[U"card_hand_space"]);
	Player player2(2, deck, object_pos[U"card_hand_size"], object_pos[U"card_hand_space"]);
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
	const float card_hand_y_space = Scene::Height() * 0.9;
	const float flag_space_size_x = Scene::Width() - (card_slot_width);
	const float flag_y = Scene::Height() / 2;
	HashTable<String, Vec2>object_pos = {
		{U"card_hand_size",Vec2(card_hand_width,card_hand_height)},
		{U"card_slot_size",Vec2(card_slot_width,card_slot_height)},
		{U"card_hand_space",Vec2(card_hand_x_space,card_hand_y_space)},
		{U"flag_space_size",Vec2(flag_space_size_x,flag_y)}
	};
	return object_pos;
}

void Main()
{
	HashTable<String, Vec2> object_pos = initializePos();
	GameState gameState = initializeGame(object_pos);

	while (System::Update())
	{
		RectF deck_card{ Arg::center((object_pos[U"card_slot_size"].x/ 2) + 10,Scene::Height() / 2),object_pos[U"card_slot_size"].x,object_pos[U"card_slot_size"].y};
		deck_card.draw(Arg::top = ColorF{ 0.5, 0.7, 0.9 }, Arg::bottom = ColorF{ 0.5, 0.9, 0.7 });

		for (int flag = 0; flag < 9; flag++)
		{
			
			float flag_between_size = (object_pos[U"flag_space_size"].x - 9 * gameState.getFlags()[flag].getTexture().width())/10;
			float flag_x = object_pos[U"card_slot_size"].x + flag_between_size+(gameState.getFlags()[flag].getTexture().width()/2) +flag* (gameState.getFlags()[flag].getTexture().width()+ flag_between_size) ;
			
			Vec2 flag_position = { flag_x, object_pos[U"flag_space_size"].y};
			gameState.getFlags()[flag].setPos(flag_position);
			gameState.getFlags()[flag].draw();
			gameState.getFlags()[flag].slotdraw();
		}

		gameState.getPlayer1()->drawHand(gameState);
	}
}
