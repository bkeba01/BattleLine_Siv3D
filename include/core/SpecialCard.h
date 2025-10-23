#ifndef SPECIALCARD_H
#define SPECIALCARD_H
#include <Siv3D.hpp>
#include "core/Common.h"
#include "core/CardBase.h"

class SpecialCard : public CardBase {
    private:
        SpecialCardType m_type;
        String m_name;
        String m_description;
		Font m_smallFont;

    public:
        SpecialCard();
        SpecialCard(SpecialCardType type, const String& name, const String& description,
                    const Font& font, const Texture& texture, const Texture& backtexture, const Font& smallfont);

        SpecialCardType getType() const { return m_type; }
        String getName() const { return m_name; }
        String getDescription() const { return m_description; }

        void draw() const override;
        void drawBack() const override;

        bool operator==(const SpecialCard& other) const {
            return m_type == other.m_type;
        }
};

#endif
