#ifndef FLAG_H
#define FLAG_H
#include <Siv3D.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <numeric>

class Player;
class Card;
class Flag {
    private:
        int m_position;
        std::vector<std::vector<Card>> m_cards;
        int m_take_flag = 2; // 0:プレイヤー0取得,1:プレイヤー1取得,2:未決定
        std::vector<int> role;
        //5:ウェッジ>4:ファランクス>3:バタリオン>2:スカーミッシャー>1:ホスト---0:なし
        bool m_card_space[2]; // 各プレイヤーのカードスペースが空かどうかを示す配列
		Texture m_texture;
		Vec2 m_card_slot_size;
		Vec2 m_draw_position;
    public:
        Flag(int position);
        int getPosition() const;
        void placeCard(const Card& card, Player* currentPlayer);
        Card getCard(int playerIndex, int slotIndex) const;
        int checkCardSpace(Player* currentPlayer);
        void checkRoleStatus(Player* currentPlayer);
        int getRoleStatus(Player *currentPlayer);
        void checkFlagStatus();
        int getFlagStatus();
        Card* getFlagCard(int playerIndex);
        void setCardSpace(bool isEmpty, int playerIndex);
        bool getCardSpace(int playerIndex);
		void setPos(Vec2 draw_position) { m_draw_position = draw_position; };
		Vec2 getPos() { return m_draw_position; };
		void draw();
		void setTexture(Texture);
		Texture getTexture();
		void setCardSlotSize(Vec2 card_slot_size) { m_card_slot_size = card_slot_size; };
		Vec2 getCardSlotSize() { return m_card_slot_size; };
		void slotdraw();
		RectF getCardSlotRect(int playerIndex, int slotIndex);

};

#endif // FLAG_H
