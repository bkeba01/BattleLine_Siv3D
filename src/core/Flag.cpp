#include "core/Flag.h"
#include "core/Slot.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/Common.h" // 追加
#include "core/GameState.h"
#include "core/SpecialCard.h"
#include "core/WeatherSlot.h"

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
    WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_slotIndex);

    // 泥カードがある場合は4枚まで、そうでない場合は3枚まで
    int maxCardCount = weatherSlot.hasMudCard() ? 4 : 3;

    // 必要な枚数分のカードが揃っているかチェック
    for (int i = 0; i < maxCardCount; ++i)
    {
        if (currentSlot.getCards()[playerIndex][i] == nullptr)
        {
            return; // まだカードが揃っていない
        }
    }

    std::vector<bool> same_color(ste_PlayerMakeNum,false);
    std::vector<bool> same_value(ste_PlayerMakeNum,false);
    std::vector<bool> serial_value(ste_PlayerMakeNum,false);
	bool tohave_special_card = false;
	for(int i=ste_SlotCardMinNum; i < maxCardCount; ++i)
	{
		// CardBaseからCardへのダウンキャスト
		if (std::dynamic_pointer_cast<Card>(currentSlot.getCards()[playerIndex][i]) == nullptr)
		{
			tohave_special_card = true;
			break;
		}

	}
	if(tohave_special_card)
	{
		checkRoleStatus_Special(gameState,currentPlayer,&currentSlot);
		return;
	}
	else
	{
		checkRoleStatus_normal(gameState,currentPlayer,&currentSlot);
		return;
	}
	/*
    // CardBaseからCardへのダウンキャスト
    auto firstCard = std::dynamic_pointer_cast<Card>(currentSlot.getCards()[playerIndex][static_cast<int>(ste_SlotCardMinNum)]);
	
    same_color[playerIndex]=std::all_of(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(),[&](const std::shared_ptr<CardBase>& c){
        auto card = std::dynamic_pointer_cast<Card>(c);
        return card && firstCard && card->getColor() == firstCard->getColor();
    });
    same_value[playerIndex]=std::all_of(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(),[&](const std::shared_ptr<CardBase>& c){
        auto card = std::dynamic_pointer_cast<Card>(c);
        return card && firstCard && card->getValue() == firstCard->getValue();
    });
    serial_value[playerIndex]=std::adjacent_find(currentSlot.getCards()[playerIndex].begin(), currentSlot.getCards()[playerIndex].end(), [](const std::shared_ptr<CardBase>& a, const std::shared_ptr<CardBase>& b){
        auto cardA = std::dynamic_pointer_cast<Card>(a);
        auto cardB = std::dynamic_pointer_cast<Card>(b);
        return !cardA || !cardB || cardB->getValue()-cardA->getValue()!=1;
    })== currentSlot.getCards()[playerIndex].end();
    
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
	*/
}

void Flag::checkRoleStatus_Special(GameState& gameState,Player* currentPlayer,Slot* currentSlot)
{
	int playerIndex = currentPlayer->getId();

	std::vector<bool> same_color(ste_PlayerMakeNum, false);
	std::vector<bool> same_value(ste_PlayerMakeNum, false);
	std::vector<bool> serial_value(ste_PlayerMakeNum, false);

	// 泥カードがある場合は4枚まで、そうでない場合は3枚まで
	WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_slotIndex);
	int maxCardCount = weatherSlot.hasMudCard() ? 4 : 3;

	// カードを値と色のペアとして扱う構造体
	struct CardInfo {
		int value;
		int color;
	};

	std::vector<CardInfo> cardInfos;

	// 全てのカード（通常カードとSpecialCard）から値と色を取得
	for (int i = ste_SlotCardMinNum; i < maxCardCount; ++i)
	{
		auto cardBase = currentSlot->getCards()[playerIndex][i];
		if (cardBase == nullptr)
		{
			continue;
		}

		// SpecialCardとして取得を試みる
		auto specialCard = std::dynamic_pointer_cast<SpecialCard>(cardBase);
		if (specialCard)
		{
			cardInfos.push_back({specialCard->getValue(), specialCard->getColor()});
		}
		else
		{
			// 通常カードとして取得
			auto normalCard = std::dynamic_pointer_cast<Card>(cardBase);
			if (normalCard)
			{
				cardInfos.push_back({normalCard->getValue(), normalCard->getColor()});
			}
		}
	}

	// カードが揃っていない場合は役なし
	if (cardInfos.size() != static_cast<size_t>(maxCardCount))
	{
		role[playerIndex] = ste_InitRole;
		return;
	}

	// 値でソート（連番チェック用）
	std::sort(cardInfos.begin(), cardInfos.end(), [](const CardInfo& a, const CardInfo& b) {
		return a.value < b.value;
	});

	// 同色チェック
	same_color[playerIndex] = std::all_of(cardInfos.begin(), cardInfos.end(), [&](const CardInfo& info) {
		return info.color == cardInfos[0].color;
	});

	// 同値チェック
	same_value[playerIndex] = std::all_of(cardInfos.begin(), cardInfos.end(), [&](const CardInfo& info) {
		return info.value == cardInfos[0].value;
	});

	// 連番チェック
	serial_value[playerIndex] = true;
	for (size_t i = 1; i < cardInfos.size(); ++i)
	{
		if (cardInfos[i].value != cardInfos[i - 1].value + 1)
		{
			serial_value[playerIndex] = false;
			break;
		}
	}

	// 役判定
	if (same_color[playerIndex] && serial_value[playerIndex])
	{
		role[playerIndex] = ste_StraightFlush;
	}
	else if (same_value[playerIndex])
	{
		// 4枚の場合はFourOfKind、3枚の場合はThreeOfKind
		role[playerIndex] = (maxCardCount == 4) ? ste_FourOfKind : ste_ThreeOfKind;
	}
	else if (same_color[playerIndex])
	{
		role[playerIndex] = ste_Flush;
	}
	else if (serial_value[playerIndex])
	{
		role[playerIndex] = ste_Straight;
	}
	else
	{
		role[playerIndex] = ste_NoneRole;
	}
}

void Flag::checkRoleStatus_normal(GameState& gameState, Player* currentPlayer,Slot* currentSlot)
{
	int playerIndex = currentPlayer->getId();

	std::vector<bool> same_color(ste_PlayerMakeNum, false);
	std::vector<bool> same_value(ste_PlayerMakeNum, false);
	std::vector<bool> serial_value(ste_PlayerMakeNum, false);

	// 泥カードがある場合は4枚まで、そうでない場合は3枚まで
	WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_slotIndex);
	int maxCardCount = weatherSlot.hasMudCard() ? 4 : 3;

	// 実際に配置されているカードの数を確認
	int actualCardCount = 0;
	for (int i = 0; i < maxCardCount; ++i)
	{
		if (currentSlot->getCards()[playerIndex][i] != nullptr)
		{
			actualCardCount++;
		}
	}

	// まだカードが揃っていない場合
	if (actualCardCount < maxCardCount)
	{
		role[playerIndex] = ste_InitRole;
		return;
	}

	// CardBaseからCardへのダウンキャスト
	auto firstCard = std::dynamic_pointer_cast<Card>(currentSlot->getCards()[playerIndex][static_cast<int>(ste_SlotCardMinNum)]);

	// maxCardCountまでのカードのみをチェック
	auto begin = currentSlot->getCards()[playerIndex].begin();
	auto end = begin + maxCardCount;

	same_color[playerIndex] = std::all_of(begin, end, [&](const std::shared_ptr<CardBase>& c) {
		auto card = std::dynamic_pointer_cast<Card>(c);
		return card && firstCard && card->getColor() == firstCard->getColor();
	});
	same_value[playerIndex] = std::all_of(begin, end, [&](const std::shared_ptr<CardBase>& c) {
		auto card = std::dynamic_pointer_cast<Card>(c);
		return card && firstCard && card->getValue() == firstCard->getValue();
	});
	serial_value[playerIndex] = std::adjacent_find(begin, end, [](const std::shared_ptr<CardBase>& a, const std::shared_ptr<CardBase>& b) {
		auto cardA = std::dynamic_pointer_cast<Card>(a);
		auto cardB = std::dynamic_pointer_cast<Card>(b);
		return !cardA || !cardB || cardB->getValue() - cardA->getValue() != 1;
	}) == end;

	if (same_color[playerIndex] && serial_value[playerIndex])
	{
		role[playerIndex] = ste_StraightFlush;
	}
	else if (same_value[playerIndex])
	{
		// 4枚の場合はFourOfKind、3枚の場合はThreeOfKind
		role[playerIndex] = (maxCardCount == 4) ? ste_FourOfKind : ste_ThreeOfKind;
	}
	else if (same_color[playerIndex])
	{
		role[playerIndex] = ste_Flush;
	}
	else if (serial_value[playerIndex])
	{
		role[playerIndex] = ste_Straight;
	}
	else
	{
		role[playerIndex] = ste_NoneRole;
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
    WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_slotIndex);

    // Fogカード（霧）の効果：役を無視して合計値のみで勝敗を決定
    if (weatherSlot.hasFogCard())
    {
        std::vector<int> total_value(ste_PlayerMakeNum, 0);

        // Player1の合計値計算
        total_value[static_cast<int>(ste_Player1)] = std::accumulate(
            currentSlot.getCards()[static_cast<int>(ste_Player1)].begin(),
            currentSlot.getCards()[static_cast<int>(ste_Player1)].end(), 0,
            [](int sum, const std::shared_ptr<CardBase>& c) {
                auto card = std::dynamic_pointer_cast<Card>(c);
                if (card) return sum + card->getValue();
                auto specialCard = std::dynamic_pointer_cast<SpecialCard>(c);
                return specialCard ? sum + specialCard->getValue() : sum;
            });

        // Player2の合計値計算
        total_value[static_cast<int>(ste_Player2)] = std::accumulate(
            currentSlot.getCards()[static_cast<int>(ste_Player2)].begin(),
            currentSlot.getCards()[static_cast<int>(ste_Player2)].end(), 0,
            [](int sum, const std::shared_ptr<CardBase>& c) {
                auto card = std::dynamic_pointer_cast<Card>(c);
                if (card) return sum + card->getValue();
                auto specialCard = std::dynamic_pointer_cast<SpecialCard>(c);
                return specialCard ? sum + specialCard->getValue() : sum;
            });

        if (total_value[static_cast<int>(ste_Player1)] > total_value[static_cast<int>(ste_Player2)])
        {
            m_take_flag = static_cast<int>(ste_Player1);
        }
        else if (total_value[static_cast<int>(ste_Player1)] < total_value[static_cast<int>(ste_Player2)])
        {
            m_take_flag = static_cast<int>(ste_Player2);
        }
        else
        {
            m_take_flag = static_cast<int>(ste_NonePlayer);
        }
        return;
    }

    // 通常の判定（役による比較）
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
        total_value[static_cast<int>(ste_Player1)]=std::accumulate(currentSlot.getCards()[static_cast<int>(ste_Player1)].begin(), currentSlot.getCards()[static_cast<int>(ste_Player1)].end(), 0, [](int sum, const std::shared_ptr<CardBase>& c){
            auto card = std::dynamic_pointer_cast<Card>(c);
            return card ? sum + card->getValue() : sum;
        });
        total_value[static_cast<int>(ste_Player2)]=std::accumulate(currentSlot.getCards()[static_cast<int>(ste_Player2)].begin(), currentSlot.getCards()[static_cast<int>(ste_Player2)].end(), 0, [](int sum, const std::shared_ptr<CardBase>& c){
            auto card = std::dynamic_pointer_cast<Card>(c);
            return card ? sum + card->getValue() : sum;
        });
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
