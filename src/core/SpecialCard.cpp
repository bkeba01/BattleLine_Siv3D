#include "core/SpecialCard.h"
#include "core/Slot.h"
#include "core/GameState.h"

SpecialCard::SpecialCard()
    : CardBase()
    , m_type(static_cast<SpecialCardType>(ste_NoneSpecialCard))
	, m_category(ste_TroopCard)
    , m_name(U"")
    , m_description(U"")
{
}

SpecialCard::SpecialCard(SpecialCardType type, SpecialCardCategory category, const String& name, const String& description,
                         const Font& font, const Texture& texture, const Texture& backtexture,const Font& smallfont)
    : CardBase(font, texture, backtexture)
    , m_type(type)
	, m_category(category)
    , m_name(name)
    , m_description(description)
	, m_smallFont(smallfont)
{
}

void SpecialCard::draw() const
{
    draw(m_rect);
}

void SpecialCard::draw(const RectF& rect) const
{
    // 特殊カード専用の描画（金色のグラデーション）
    rect.draw(Arg::top = ColorF{ 0.9, 0.7, 0.3 }, Arg::bottom = ColorF{ 0.8, 0.5, 0.2 });
    rect.drawFrame(3, 0, Palette::Gold);

    // アイコンまたはテクスチャ
    if (m_texture)
    {
        m_texture.resized(rect.w * 0.4).drawAt(rect.center().movedBy(0, -rect.h * 0.15));
    }

    m_smallFont(m_name).drawAt(rect.center().movedBy(0, rect.h * 0.25), Palette::Black);
}

void SpecialCard::drawBack() const
{
    m_rect.draw(Arg::top = ColorF{ 0.7, 0.5, 0.9 }, Arg::bottom = ColorF{ 0.5, 0.3, 0.7 });
    m_rect.drawFrame(3, 0, Palette::Purple);
    if (m_back_texture)
    {
        m_back_texture.resized(45).drawAt(m_rect.center());
    }
}

int containsCombination(const std::vector<int>& arr)
{
	bool has1 = std::find(arr.begin(), arr.end(), 1) != arr.end();
	bool has2 = std::find(arr.begin(), arr.end(), 2) != arr.end();
	bool has3 = std::find(arr.begin(), arr.end(), 3) != arr.end();

	if (has1 && has2)
	{
		return 3;
	}
	else if(has1 && has3)
	{
		return 2;
	}
	else if(has2 && has3)
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}


void SpecialCard::doEffect(GameState* gameState,Slot* currentSlot) const
{
	std::vector<Card> normalCards;
	std::vector<SpecialCard> specialCards;
	for (int i = ste_SlotCardMinNum; i < ste_SlotCardMaxNum; i++)
	{
		auto cardBase = currentSlot->getCards()[gameState->getCurrentPlayer()->getId()][i];
		if (cardBase == nullptr)
		{
			continue;
		}

		auto specialCardPtr = std::dynamic_pointer_cast<SpecialCard>(cardBase);
		if (specialCardPtr)
		{
			specialCards.push_back(*specialCardPtr);
		}
		else
		{
			auto normalCardPtr = std::dynamic_pointer_cast<Card>(cardBase);
			if (normalCardPtr)
			{
				normalCards.push_back(*normalCardPtr);
			}
		}
	}
	int normalCard_count = normalCards.size();
	int specialCard_count = specialCards.size();
	bool same_color_normal = false;
	bool sirial_value_normal = false;
	bool same_value_normal = false;
	if (normalCard_count == 2)
	{
		int firstColor = normalCards[0].getColor();
		int secondColor = normalCards[1].getColor();
		if (firstColor == secondColor)
		{
			same_color_normal = true;
		}
		else
		{
			same_color_normal = false;
		}
		int firstValue = normalCards[0].getValue();
		int secondValue = normalCards[1].getValue();
		if (1<=std::abs(firstValue-secondValue) && std::abs(firstValue-secondValue)<=2)
		{
			sirial_value_normal = true;
		}
		else
		{
			sirial_value_normal = false;
		}
		if (firstValue == secondValue)
		{
			same_value_normal = true;
		}
		else
		{
			same_value_normal = false;
		}
	}
	
		
	switch (m_type)
	{
	case ste_WildCard:
		if (sirial_value_normal && same_color_normal)
		{
			if(std::abs(normalCards[0].getValue()-normalCards[1].getValue())==2)
			{
				m_value=(normalCards[0].getValue() + normalCards[1].getValue())/2;
			}
			else
			{
				if(normalCards[0].getValue()<normalCards[1].getValue())
				{
					if (normalCards[1].getValue() == 10)
					{
						m_value = normalCards[0].getValue() - 1;
					}
					else
					{
						m_value = normalCards[1].getValue() + 1;
					}

				}
				else
				{
					if(normalCards[0].getValue()==10)
					{
						m_value=normalCards[1].getValue()-1;
					}
					else
					{
						m_value=normalCards[0].getValue()+1;
					}
				}
			}
			m_color = normalCards[0].getColor();
		}
		else if (same_value_normal)
		{
			m_value = normalCards[0].getValue();
			m_color = normalCards[0].getColor();
		}
		else if (same_color_normal)
		{
			m_value = ste_CardMaxValue;
			m_color = normalCards[0].getColor();
		}
		else if (sirial_value_normal)
		{
			if (std::abs(normalCards[0].getValue() - normalCards[1].getValue()) == 2)
			{
				m_value = (normalCards[0].getValue() + normalCards[1].getValue()) / 2;
			}
			else
			{
				if (normalCards[0].getValue() < normalCards[1].getValue())
				{
					if (normalCards[1].getValue() == 10)
					{
						m_value = normalCards[0].getValue() - 1;
					}
					else
					{
						m_value = normalCards[1].getValue() + 1;
					}

				}
				else
				{
					if (normalCards[0].getValue() == 10)
					{
						m_value = normalCards[1].getValue() - 1;
					}
					else
					{
						m_value = normalCards[0].getValue() + 1;
					}
				}
			}
			m_color = normalCards[0].getColor();
		}
		else
		{
			m_value = ste_CardMaxValue;
			// normalCardsが存在する場合のみ色を取得
			if (normalCard_count > 0)
			{
				m_color = normalCards[0].getColor();
			}
			else
			{
				m_color = ste_ColorMinValue; // デフォルト色
			}
		}
		break;
	case ste_WildCard_Eight:
		m_value = 8;
		// normalCardsが存在する場合のみ色を取得
		if (normalCard_count > 0)
		{
			m_color = normalCards[0].getColor();
		}
		else
		{
			m_color = ste_ColorMinValue; // デフォルト色
		}
		break;
	case ste_WildCard_Shield:
		// normalCardsが存在する場合のみ色を取得
		if (normalCard_count > 0)
		{
			m_color = normalCards[0].getColor();
		}
		else
		{
			m_color = ste_ColorMinValue; // デフォルト色
		}

		if (sirial_value_normal)
		{
			std::vector<int> normalValues;
			for (const auto& card : normalCards)
			{
				normalValues.push_back(card.getValue());
			}
			int result = containsCombination(normalValues);
			if(result)
			{
				m_value= result;
			}
			else
			{
				m_value=3;
			}
		}
		else if (same_value_normal)
		{
			if (normalCards[0].getValue() <= 3 && normalCards[0].getValue() >= 1)
			{
				m_value=normalCards[0].getValue();
			}
			else
			{
				m_value = 3;
			}
		}
		else
		{
			m_value = 3;
		}
		break;
	default:
		break;
	}
}
