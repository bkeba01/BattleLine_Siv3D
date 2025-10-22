#include "core/Flag.h"
#include "core/Slot.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/Common.h" // 追加
#include "core/GameState.h"



Flag::Flag(int position) : m_position(position) 
{
    role.resize(ste_PlayerMakeNum);
    std::fill(role.begin(), role.end(), ste_InitRole);
}

int Flag::getPosition() const{ return m_position; }

void Flag::checkRoleStatus(GameState& gameState, Player* currentPlayer) // シグネチャ更新
{
    int playerIndex = currentPlayer->getId();
    
    // GameState と m_slotIndex を使用して Slot オブジェクトを取得
    Slot& currentSlot = gameState.getSlot(m_slotIndex);

    if (std::find(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(), Card()) != currentSlot.getCards()[playerIndex].end())
    {
        return;
    }
    
    std::vector<bool> same_color(ste_PlayerMakeNum,false);
    std::vector<bool> same_value(ste_PlayerMakeNum,false);
    std::vector<bool> serial_value(ste_PlayerMakeNum,false);
    
    same_color[playerIndex]=std::all_of(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(),[&](const Card& c){ return c.getColor() == currentSlot.getCards()[playerIndex][static_cast<int>(ste_SlotCardMinNum)].getColor();});
    same_value[playerIndex]=std::all_of(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(),[&](const Card& c){ return c.getValue() == currentSlot.getCards()[playerIndex][static_cast<int>(ste_SlotCardMinNum)].getValue();});
    serial_value[playerIndex]=std::adjacent_find(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(), [](const Card& a,const Card& b){return b.getValue()-a.getValue()!=1;})== currentSlot.getCards()[playerIndex].end();
    
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
    else if(currentSlot.checkCardSpace(currentPlayer)== static_cast<int>(ste_SlotCard_NonSpace))
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

void Flag::checkFlagStatus(GameState& gameState)
{
    if(role[static_cast<int>(ste_Player1)]==ste_InitRole || role[static_cast<int>(ste_Player2)]== ste_InitRole)
    {
        return;
    }
    // GameState と m_slotIndex を使用して Slot オブジェクトを取得
    Slot& currentSlot = gameState.getSlot(m_slotIndex);

    if(role[static_cast<int>(ste_Player1)]>role[static_cast<int>(ste_Player2)])
    {
        m_take_flag= static_cast<int>(ste_Player1);
    }
    else if(role[static_cast<int>(ste_Player1)]<role[static_cast<int>(ste_Player2)])
    {
        m_take_flag= static_cast<int>(ste_Player2);
    }
    else if(role[static_cast<int>(ste_Player1)]==role[static_cast<int>(ste_Player2)])
    {
        std::vector<int> total_value(ste_PlayerMakeNum,0);
        total_value[static_cast<int>(ste_Player1)]=std::accumulate(currentSlot.getCards()[static_cast<int>(ste_Player1)].begin(), currentSlot.getCards()[static_cast<int>(ste_Player1)].end(), 0, [](int sum,const Card& c){return sum+c.getValue();});
        total_value[static_cast<int>(ste_Player2)]=std::accumulate(currentSlot.getCards()[static_cast<int>(ste_Player2)].begin(), currentSlot.getCards()[static_cast<int>(ste_Player2)].end(), 0, [](int sum,const Card& c){return sum+c.getValue();});
        if(total_value[static_cast<int>(ste_Player1)] > total_value[static_cast<int>(ste_Player2)])
        {
            m_take_flag= static_cast<int>(ste_Player1);
        }
        else if(total_value[static_cast<int>(ste_Player1)] < total_value[static_cast<int>(ste_Player2)])
        {
            m_take_flag= static_cast<int>(ste_Player2);
        }
        else
        {
            m_take_flag=static_cast<int>(ste_NonePlayer);
        }
    }
    else
    {
        m_take_flag= static_cast<int>(ste_NonePlayer);
    }
}

int Flag::getFlagStatus()
{
    return m_take_flag;
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

void Flag::drawWinnerFlag(GameState& gamestate)
{
	if (m_take_flag == static_cast<int>(ste_NonePlayer))
	{
		return;
		m_texture.drawAt(m_draw_position.x,m_draw_position.y);
	}
	else if (m_take_flag== gamestate.getCurrentPlayer()->getId())
	{
		m_texture.drawAt(m_draw_position.x, m_draw_position.y+(gamestate.getSlot(m_position).getCardSlotSize().y));
	}
	else if (m_take_flag == gamestate.getOpponentPlayer()->getId())
	{
		m_texture.drawAt(m_draw_position.x, m_draw_position.y - (gamestate.getSlot(m_position).getCardSlotSize().y));
	}
}
