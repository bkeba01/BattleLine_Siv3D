#ifndef FLAG_H
#define FLAG_H
#include <Siv3D.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <numeric>
#include "core/Common.h"

class Player;
class Card;
class Slot;
class GameState;

class Flag {
    private:
        int m_position;
        int m_take_flag = ste_NonePlayer; // 0:プレイヤー0取得,1:プレイヤー1取得,2:未決定
        std::vector<int> role;
        //5:ウェッジ>4:ファランクス>3:バタリオン>2:スカーミッシャー>1:ホスト---0:なし
		Texture m_texture;
		Vec2 m_draw_position;
		int m_slotIndex = -1;
    public:
        Flag(int position);
        int getPosition() const;
        void checkRoleStatus(GameState& gameState, Player* currentPlayer);
        int getRoleStatus(Player *currentPlayer);
        void checkFlagStatus(GameState& gameState);
        int getFlagStatus();
		void setPos(Vec2 draw_position) { m_draw_position = draw_position; };
		Vec2 getPos() { return m_draw_position; };
		void draw();
		void setTexture(Texture);
		Texture getTexture();
		void setSlotIndex(int index) { m_slotIndex = index; };
		int getSlotIndex() const { return m_slotIndex; };

};

#endif // FLAG_H
