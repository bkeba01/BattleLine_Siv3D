#include "core/Flag.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/Common.h"


Flag::Flag(int position) : m_position(position) 
{
    m_cards.resize(ste_PlayerMakeNum);
    m_cards[ste_Player1].resize(ste_SlotCardMakeNum);
    m_cards[ste_Player2].resize(ste_SlotCardMakeNum);
    role.resize(ste_PlayerMakeNum);
    std::fill(role.begin(), role.end(), ste_InitRole);
}

int Flag::getPosition() const{ return m_position; }

void Flag::placeCard(const Card& card, Player* currentPlayer)
{
    int playerIndex = currentPlayer->getId();
    Card cardToPlace = card;

    m_cards[playerIndex][Flag::checkCardSpace(currentPlayer)] = cardToPlace;

    Flag::checkRoleStatus(currentPlayer);
    Flag::checkFlagStatus();
}

void Flag::checkRoleStatus(Player* currentPlayer) 
{
    int playerIndex = currentPlayer->getId();
    if (std::find(m_cards[playerIndex].begin(), m_cards[playerIndex].end(), Card()) != m_cards[playerIndex].end()) 
    {
        return;
    }
    
    std::vector<bool> same_color(ste_PlayerMakeNum,false);
    std::vector<bool> same_value(ste_PlayerMakeNum,false);
    std::vector<bool> serial_value(ste_PlayerMakeNum,false);
    
    same_color[playerIndex]=std::all_of(m_cards[playerIndex].begin(), m_cards[playerIndex].end(),[&](const Card& c){ return c.getColor() == m_cards[playerIndex][ste_SlotCardMinNum].getColor();});
    same_value[playerIndex]=std::all_of(m_cards[playerIndex].begin(), m_cards[playerIndex].end(),[&](const Card& c){ return c.getValue() == m_cards[playerIndex][ste_SlotCardMinNum].getValue();});
    serial_value[playerIndex]=std::adjacent_find(m_cards[playerIndex].begin(), m_cards[playerIndex].end(), [](const Card& a,const Card& b){return b.getValue()-a.getValue()!=1;})==m_cards[playerIndex].end();
    
    if(same_color[playerIndex]&&serial_value[playerIndex])
    {
        role[playerIndex]=ste_StraightFlush;
    }
    else if(same_value[playerIndex])
    {
        role[playerIndex]=ste_ThreeOfKind;
    }
    else if(same_color[playerIndex])
    {
        role[playerIndex]=ste_Flush;
    }
    else if(serial_value[playerIndex])
    {
        role[playerIndex]=ste_Straight;
    }
    else if(Flag::checkCardSpace(currentPlayer)!=-1)
    {
        role[playerIndex]=ste_NoneRole;
    }
	else
	{
		role[playerIndex] = ste_InitRole;
	}
}

int Flag::getRoleStatus(Player* currentPlayer)
{
    return role[currentPlayer->getId()];
}

void Flag::checkFlagStatus()
{
    if(role[ste_Player1]==ste_InitRole || role[ste_Player2]== ste_InitRole)
    {
        return;
    }
    if(role[ste_Player1]>role[ste_Player2])
    {
        m_take_flag= ste_Player1;
    }
    else if(role[ste_Player1]<role[ste_Player2])
    {
        m_take_flag= ste_Player2;
    }
    else if(role[ste_Player1]==role[ste_Player2])
    {
        std::vector<int> total_value(ste_PlayerMakeNum,0);
        total_value[ste_Player1]=std::accumulate(m_cards[ste_Player1].begin(), m_cards[ste_Player1].end(), 0, [](int sum,const Card& c){return sum+c.getValue();});
        total_value[ste_Player2]=std::accumulate(m_cards[ste_Player2].begin(), m_cards[ste_Player2].end(), 0, [](int sum,const Card& c){return sum+c.getValue();});
        if(total_value[ste_Player1] > total_value[ste_Player2])
        {
            m_take_flag= ste_Player1;
        }
        else if(total_value[ste_Player1] < total_value[ste_Player2])
        {
            m_take_flag= ste_Player2;
        }
        else
        {
            m_take_flag=ste_NonPlayer;
        }
    }
    else
    {
        m_take_flag= ste_NonPlayer;
    }
}

int Flag::getFlagStatus()
{
    return m_take_flag;
}

int Flag::checkCardSpace(Player* currentPlayer) 
{
    int playerIndex = currentPlayer->getId();
    for (int i = 0; i < 3; ++i) 
    {
        if (m_cards[playerIndex][i].getValue() == ste_NonCard) 
        {
            return i; // 空きスロットのインデックスを返す
        }
    }

    return -1;
}

void Flag::setCardSpace(bool isEmpty, int playerIndex)
{
    m_card_space[playerIndex] = isEmpty;
}

bool Flag::getCardSpace(int playerIndex)
{
    if (playerIndex < ste_PlayerMin || playerIndex > ste_PlayerMax) 
    {
        throw std::out_of_range("Invalid player index");
    }
    return m_card_space[playerIndex];
}

Card Flag::getCard(int playerIndex, int slotIndex) const
{
    if (playerIndex < ste_PlayerMin || playerIndex > ste_PlayerMax || slotIndex < 0 || slotIndex >= 3) 
    {
        throw std::out_of_range("Invalid player or slot index");
    }
    return m_cards[playerIndex][slotIndex];
}

Card* Flag::getFlagCard(int playerIndex)
{
    return m_cards[playerIndex].data();
}

void Flag::draw()
{
	m_texture.drawAt(m_draw_position);
}
void Flag::setTexture(Texture texture)
{
	m_texture = texture;
}
Texture Flag::getTexture()
{
	return m_texture;
}

void Flag::slotdraw()
{
    // Player 0 (相手側・上)
    for (int i = 0; i < m_cards[ste_Player1].size(); ++i)
    {
        const RectF rect=getCardSlotRect(0, i);
        const Card& card = m_cards[ste_Player1][i];

        if (card.getValue() != 0)
        {
            card.draw(rect);
        }
        else
        {
            rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
            float thickness = 4.0;
            ColorF lineColor = Palette::White;
            rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
        }
    }

    // Player 1 (自分側・下)
    for (int i = 0; i < m_cards[ste_Player2].size(); ++i)
    {
		const RectF rect=getCardSlotRect(1, i);
        const Card& card = m_cards[ste_Player2][i];

        if (card.getValue() != 0)
        {
            card.draw(rect);
        }
        else
        {
            rect.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
            float thickness = 4.0;
            ColorF lineColor = Palette::White;
            rect.top().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.right().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.bottom().draw(LineStyle::SquareDot, thickness, lineColor);
            rect.left().draw(LineStyle::SquareDot, thickness, lineColor);
        }
    }
}

RectF Flag::getCardSlotRect(int playerIndex, int slotIndex)
{

	RectF rect;
	if (playerIndex == ste_Player1) // プレイヤー1のカードスロット
	{
		rect={ m_draw_position.x - (m_card_slot_size.x / 2), m_draw_position.y - (m_texture.height() / 2) - m_card_slot_size.y - 20 - (slotIndex * (m_card_slot_size.y / 3)), m_card_slot_size.x, m_card_slot_size.y };
	}
	else if (playerIndex == ste_Player2) // プレイヤー2のカードスロット
	{
		rect={ m_draw_position.x - (m_card_slot_size.x / 2), m_texture.height() / 2 + m_draw_position.y + 20 + slotIndex * (m_card_slot_size.y / 3), m_card_slot_size.x, m_card_slot_size.y };
	}
	return rect;
}
