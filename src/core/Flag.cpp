#include "core/Flag.h"
#include "core/Slot.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/Common.h" // 追加
#include "core/GameState.h"
#include "core/SpecialCard.h"
#include "core/WeatherSlot.h"
#include <algorithm>
#include <vector>

// ワイルドカード最適化のための構造体
struct CardInfo {
	int value;
	int color;
	bool isWild;
	SpecialCardType wildType;

	CardInfo(int v, int c, bool wild = false, SpecialCardType type = ste_WildCard)
		: value(v), color(c), isWild(wild), wildType(type) {}
};

// ワイルドカードが取れる値の範囲を取得
std::vector<int> getPossibleValues(SpecialCardType wildType) {
	switch (wildType) {
		case ste_WildCard_Eight:
			return {8};
		case ste_WildCard_Shield:
			return {1, 2, 3};
		case ste_WildCard:
		default:
			return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	}
}

// ストレートフラッシュを試す（最優先）
bool tryAssignStraightFlush(std::vector<CardInfo>& cards, int maxCardCount, int& outRole) {
	std::vector<CardInfo> normals, wilds;
	for (auto& c : cards) {
		if (c.isWild) wilds.push_back(c);
		else normals.push_back(c);
	}

	// ワイルドカードのみの場合：デフォルトで最強のストレートフラッシュ
	if (normals.empty()) {
		int startValue = 11 - maxCardCount; // 3枚なら8,9,10 / 4枚なら7,8,9,10
		for (int i = 0; i < maxCardCount; ++i) {
			cards[i].color = 1; // 赤
			cards[i].value = startValue + i;
		}
		outRole = ste_StraightFlush;
		return true;
	}

	// 通常カードから色を決定
	int targetColor = normals[0].color;

	// 通常カードの値を収集
	std::vector<int> normalValues;
	for (auto& nc : normals) {
		if (nc.color == targetColor) {
			normalValues.push_back(nc.value);
		}
	}

	// 色が統一できない場合は失敗
	if (normalValues.size() != normals.size()) {
		return false;
	}

	// 通常カードの値をソート
	std::sort(normalValues.begin(), normalValues.end());

	// ワイルドカードで連番を作れるか試す
	int wildCount = wilds.size();

	// 最小値から連番を作る戦略
	int minVal = normalValues[0];
	int maxVal = normalValues.back();

	// パターン1: 最小値から上に伸ばす（例: 5,6,7 or 5,7 → 5,6,7）
	std::vector<int> pattern;
	for (int start = std::max(1, minVal - wildCount); start <= minVal; ++start) {
		pattern.clear();
		for (int v = start; v < start + maxCardCount; ++v) {
			if (v < 1 || v > 10) break;
			pattern.push_back(v);
		}

		if (pattern.size() != static_cast<size_t>(maxCardCount)) continue;

		// このパターンで通常カードがすべて含まれているかチェック
		bool allIncluded = true;
		for (int nv : normalValues) {
			if (std::find(pattern.begin(), pattern.end(), nv) == pattern.end()) {
				allIncluded = false;
				break;
			}
		}

		if (!allIncluded) continue;

		// ワイルドカードで埋められるかチェック
		int wildIdx = 0;
		bool canFill = true;
		for (int pv : pattern) {
			bool found = false;
			for (int nv : normalValues) {
				if (nv == pv) {
					found = true;
					break;
				}
			}
			if (!found) {
				// ワイルドカードで埋める必要がある
				if (wildIdx >= wildCount) {
					canFill = false;
					break;
				}
				auto possibleValues = getPossibleValues(wilds[wildIdx].wildType);
				if (std::find(possibleValues.begin(), possibleValues.end(), pv) == possibleValues.end()) {
					canFill = false;
					break;
				}
				wildIdx++;
			}
		}

		if (canFill && wildIdx == wildCount) {
			// 成功！カードに割り当て
			int cardIdx = 0;
			wildIdx = 0;
			for (int pv : pattern) {
				bool isNormal = false;
				for (auto& c : cards) {
					if (!c.isWild && c.value == pv && c.color == targetColor) {
						isNormal = true;
						break;
					}
				}
				if (!isNormal) {
					// ワイルドカードを割り当て
					while (cardIdx < cards.size() && !cards[cardIdx].isWild) cardIdx++;
					if (cardIdx < cards.size()) {
						cards[cardIdx].value = pv;
						cards[cardIdx].color = targetColor;
						cardIdx++;
					}
				}
			}
			outRole = ste_StraightFlush;
			return true;
		}
	}

	return false;
}

// フォーカード/スリーカードを試す
bool tryAssignSameValue(std::vector<CardInfo>& cards, int maxCardCount, int targetCount, int& outRole) {
	std::vector<CardInfo> normals, wilds;
	for (auto& c : cards) {
		if (c.isWild) wilds.push_back(c);
		else normals.push_back(c);
	}

	// ワイルドカードのみの場合
	if (normals.empty()) {
		int targetValue = 10; // デフォルトで最大値
		for (auto& c : cards) {
			auto possibleValues = getPossibleValues(c.wildType);
			if (std::find(possibleValues.begin(), possibleValues.end(), targetValue) != possibleValues.end()) {
				c.value = targetValue;
				c.color = 1;
			} else {
				c.value = possibleValues.back(); // 制約内の最大値
				c.color = 1;
			}
		}
		outRole = (targetCount == 4) ? ste_FourOfKind : ste_ThreeOfKind;
		return true;
	}

	// 通常カードから値を決定
	int targetValue = normals[0].value;

	// ワイルドカードで埋められるかチェック
	int colorIdx = 1;
	for (auto& c : cards) {
		if (c.isWild) {
			auto possibleValues = getPossibleValues(c.wildType);
			if (std::find(possibleValues.begin(), possibleValues.end(), targetValue) != possibleValues.end()) {
				c.value = targetValue;
				c.color = colorIdx++;
				if (colorIdx > 6) colorIdx = 1;
			} else {
				return false; // ワイルドカードの制約で同じ値にできない
			}
		}
	}

	outRole = (targetCount == 4) ? ste_FourOfKind : ste_ThreeOfKind;
	return true;
}

// フラッシュを試す
bool tryAssignFlush(std::vector<CardInfo>& cards, int maxCardCount, int& outRole) {
	std::vector<CardInfo> normals, wilds;
	for (auto& c : cards) {
		if (c.isWild) wilds.push_back(c);
		else normals.push_back(c);
	}

	// 通常カードから色を決定
	int targetColor = normals.empty() ? 1 : normals[0].color;

	// 色が統一できるかチェック
	for (auto& nc : normals) {
		if (nc.color != targetColor) {
			return false;
		}
	}

	// ワイルドカードに高い値を割り当て
	int valueIdx = 10;
	for (auto& c : cards) {
		if (c.isWild) {
			auto possibleValues = getPossibleValues(c.wildType);
			// できるだけ高い値を選ぶ（通常カードと重複しないように）
			bool found = false;
			for (int v = 10; v >= 1; --v) {
				if (std::find(possibleValues.begin(), possibleValues.end(), v) != possibleValues.end()) {
					bool duplicate = false;
					for (auto& nc : normals) {
						if (nc.value == v) {
							duplicate = true;
							break;
						}
					}
					if (!duplicate) {
						c.value = v;
						c.color = targetColor;
						found = true;
						break;
					}
				}
			}
			if (!found) {
				c.value = possibleValues.back();
				c.color = targetColor;
			}
		}
	}

	outRole = ste_Flush;
	return true;
}

// ストレートを試す
bool tryAssignStraight(std::vector<CardInfo>& cards, int maxCardCount, int& outRole) {
	// ストレートフラッシュと同じロジックだが、色の制約なし
	std::vector<CardInfo> normals, wilds;
	for (auto& c : cards) {
		if (c.isWild) wilds.push_back(c);
		else normals.push_back(c);
	}

	if (normals.empty()) {
		int startValue = 11 - maxCardCount;
		for (int i = 0; i < maxCardCount; ++i) {
			cards[i].color = (i % 6) + 1;
			cards[i].value = startValue + i;
		}
		outRole = ste_Straight;
		return true;
	}

	std::vector<int> normalValues;
	for (auto& nc : normals) {
		normalValues.push_back(nc.value);
	}
	std::sort(normalValues.begin(), normalValues.end());

	int wildCount = wilds.size();
	int minVal = normalValues[0];

	for (int start = std::max(1, minVal - wildCount); start <= minVal; ++start) {
		std::vector<int> pattern;
		for (int v = start; v < start + maxCardCount; ++v) {
			if (v < 1 || v > 10) break;
			pattern.push_back(v);
		}

		if (pattern.size() != static_cast<size_t>(maxCardCount)) continue;

		bool allIncluded = true;
		for (int nv : normalValues) {
			if (std::find(pattern.begin(), pattern.end(), nv) == pattern.end()) {
				allIncluded = false;
				break;
			}
		}

		if (!allIncluded) continue;

		int wildIdx = 0;
		bool canFill = true;
		for (int pv : pattern) {
			bool found = false;
			for (int nv : normalValues) {
				if (nv == pv) {
					found = true;
					break;
				}
			}
			if (!found) {
				if (wildIdx >= wildCount) {
					canFill = false;
					break;
				}
				auto possibleValues = getPossibleValues(wilds[wildIdx].wildType);
				if (std::find(possibleValues.begin(), possibleValues.end(), pv) == possibleValues.end()) {
					canFill = false;
					break;
				}
				wildIdx++;
			}
		}

		if (canFill && wildIdx == wildCount) {
			int cardIdx = 0;
			wildIdx = 0;
			int colorIdx = 1;
			for (int pv : pattern) {
				bool isNormal = false;
				for (auto& c : cards) {
					if (!c.isWild && c.value == pv) {
						isNormal = true;
						break;
					}
				}
				if (!isNormal) {
					while (cardIdx < cards.size() && !cards[cardIdx].isWild) cardIdx++;
					if (cardIdx < cards.size()) {
						cards[cardIdx].value = pv;
						cards[cardIdx].color = colorIdx++;
						if (colorIdx > 6) colorIdx = 1;
						cardIdx++;
					}
				}
			}
			outRole = ste_Straight;
			return true;
		}
	}

	return false;
}

// 役なしで最大値を割り当て
void assignMaxValue(std::vector<CardInfo>& cards, int maxCardCount) {
	for (auto& c : cards) {
		if (c.isWild) {
			auto possibleValues = getPossibleValues(c.wildType);
			c.value = possibleValues.back(); // 最大値
			c.color = 1;
		}
	}
}

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

	// 泥カードがある場合は4枚まで、そうでない場合は3枚まで
	WeatherSlot& weatherSlot = gameState.getWeatherSlot(m_slotIndex);
	int maxCardCount = weatherSlot.hasMudCard() ? 4 : 3;

	// カード情報を収集
	std::vector<CardInfo> cardInfos;
	std::vector<std::shared_ptr<SpecialCard>> wildCardPtrs; // ワイルドカードへのポインタを保存

	for (int i = ste_SlotCardMinNum; i < maxCardCount; ++i)
	{
		auto cardBase = currentSlot->getCards()[playerIndex][i];
		if (cardBase == nullptr)
		{
			continue;
		}

		// SpecialCardかチェック
		auto specialCard = std::dynamic_pointer_cast<SpecialCard>(cardBase);
		if (specialCard)
		{
			// ワイルドカード系かチェック
			SpecialCardType stype = specialCard->getType();
			if (stype == ste_WildCard || stype == ste_WildCard_Eight || stype == ste_WildCard_Shield)
			{
				cardInfos.push_back(CardInfo(0, 0, true, stype));
				wildCardPtrs.push_back(specialCard);
			}
			else
			{
				// ワイルドカードではない特殊カード（既に値と色が設定されている）
				cardInfos.push_back(CardInfo(specialCard->getValue(), specialCard->getColor(), false));
			}
		}
		else
		{
			// 通常カード
			auto normalCard = std::dynamic_pointer_cast<Card>(cardBase);
			if (normalCard)
			{
				cardInfos.push_back(CardInfo(normalCard->getValue(), normalCard->getColor(), false));
			}
		}
	}

	// カードが揃っていない場合は役なし
	if (cardInfos.size() != static_cast<size_t>(maxCardCount))
	{
		role[playerIndex] = ste_InitRole;
		return;
	}

	// ヒューリスティック: 役の優先順位で試す
	// 泥カードで4枚の場合の優先順位:
	//   フォーカード > フラッシュ(4枚) > ストレート(4枚) > スリーカード
	// 3枚の場合の優先順位:
	//   スリーカード > フラッシュ(3枚) > ストレート(3枚)
	int assignedRole = ste_NoneRole;

	// 1. ストレートフラッシュを試す（最強）
	if (tryAssignStraightFlush(cardInfos, maxCardCount, assignedRole))
	{
		// 成功：ワイルドカードに値と色を設定
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 2. フォーカードを試す（4枚の場合のみ）
	if (maxCardCount == 4 && tryAssignSameValue(cardInfos, maxCardCount, 4, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 3. フラッシュを試す（4枚の場合のみ、ストレート4枚より強い）
	if (maxCardCount == 4 && tryAssignFlush(cardInfos, maxCardCount, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 4. ストレートを試す（4枚の場合のみ、スリーカードより強い）
	if (maxCardCount == 4 && tryAssignStraight(cardInfos, maxCardCount, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 5. スリーカードを試す
	if (tryAssignSameValue(cardInfos, maxCardCount, maxCardCount, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 6. フラッシュを試す（3枚の場合のみ、4枚の場合は既に試した）
	if (maxCardCount == 3 && tryAssignFlush(cardInfos, maxCardCount, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 7. ストレートを試す（3枚の場合のみ、4枚の場合は既に試した）
	if (maxCardCount == 3 && tryAssignStraight(cardInfos, maxCardCount, assignedRole))
	{
		int wildIdx = 0;
		for (size_t i = 0; i < cardInfos.size(); ++i)
		{
			if (cardInfos[i].isWild)
			{
				wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
				wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
				wildIdx++;
			}
		}
		role[playerIndex] = assignedRole;
		return;
	}

	// 8. 役なし：最大値を割り当て
	assignMaxValue(cardInfos, maxCardCount);
	int wildIdx = 0;
	for (size_t i = 0; i < cardInfos.size(); ++i)
	{
		if (cardInfos[i].isWild)
		{
			wildCardPtrs[wildIdx]->setValue(cardInfos[i].value);
			wildCardPtrs[wildIdx]->setColor(cardInfos[i].color);
			wildIdx++;
		}
	}
	role[playerIndex] = ste_NoneRole;
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

	// カード情報を収集（連番判定のためソートが必要）
	struct CardInfo {
		int value;
		int color;
	};

	std::vector<CardInfo> cardInfos;
	for (int i = 0; i < maxCardCount; ++i)
	{
		auto card = std::dynamic_pointer_cast<Card>(currentSlot->getCards()[playerIndex][i]);
		if (card)
		{
			cardInfos.push_back({card->getValue(), card->getColor()});
		}
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
            if (card) return sum + card->getValue();
            auto specialCard = std::dynamic_pointer_cast<SpecialCard>(c);
            return specialCard ? sum + specialCard->getValue() : sum;
        });
        total_value[static_cast<int>(ste_Player2)]=std::accumulate(currentSlot.getCards()[static_cast<int>(ste_Player2)].begin(), currentSlot.getCards()[static_cast<int>(ste_Player2)].end(), 0, [](int sum, const std::shared_ptr<CardBase>& c){
            auto card = std::dynamic_pointer_cast<Card>(c);
            if (card) return sum + card->getValue();
            auto specialCard = std::dynamic_pointer_cast<SpecialCard>(c);
            return specialCard ? sum + specialCard->getValue() : sum;
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
		m_texture.drawAt(m_draw_position.x,m_draw_position.y);
	}
	else
	{
		// 画面下側に表示されているプレイヤーのインデックスを取得
		// マルチプレイ：ローカルプレイヤー固定
		// ローカルプレイ：現在のターンプレイヤー
		int viewPlayerIndex;
		if (gamestate.isMultiplayer())
		{
			viewPlayerIndex = gamestate.getLocalPlayerIndex();
		}
		else
		{
			// ローカルプレイでは現在のターンプレイヤーが下側に表示される
			viewPlayerIndex = gamestate.getCurrentPlayer()->getId();
		}

		if (m_take_flag == viewPlayerIndex)
		{
			// 画面下側のプレイヤーが勝った場合、下に描画
			m_texture.drawAt(m_draw_position.x, m_draw_position.y + (gamestate.getSlot(m_position).getCardSlotSize().y));
		}
		else
		{
			// 画面上側のプレイヤーが勝った場合、上に描画
			m_texture.drawAt(m_draw_position.x, m_draw_position.y - (gamestate.getSlot(m_position).getCardSlotSize().y));
		}
	}
}
