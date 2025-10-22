#include "core/SpecialCard.h"

SpecialCard::SpecialCard()
    : m_type(static_cast<SpecialCardType>(ste_NoneSpecialCard))
    , m_name(U"")
    , m_description(U"")
{
}

SpecialCard::SpecialCard(SpecialCardType type, const String& name, const String& description,
                         const Font& font, const Texture& texture, const Texture& backtexture)
    : m_type(type)
    , m_name(name)
    , m_description(description)
    , m_font(font)
    , m_texture(texture)
    , m_back_texture(backtexture)
{
}

void SpecialCard::draw() const
{
    // 特殊カード専用の描画（金色のグラデーション）
    m_rect.draw(Arg::top = ColorF{ 0.9, 0.7, 0.3 }, Arg::bottom = ColorF{ 0.8, 0.5, 0.2 });
    m_rect.drawFrame(3, 0, Palette::Gold);

    // アイコンまたはテクスチャ
    if (m_texture)
    {
        m_texture.resized(m_rect.w * 0.4).drawAt(m_rect.center().movedBy(0, -m_rect.h * 0.15));
    }

    // カード名（小さめのフォント）
    Font smallFont{ 12, Typeface::Bold };
    smallFont(m_name).drawAt(m_rect.center().movedBy(0, m_rect.h * 0.25), Palette::Black);
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
