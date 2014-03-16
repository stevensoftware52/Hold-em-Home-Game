#ifndef _CARD_H
#define _CARD_H

#include "stdafx.h"

// Scale of a rendered card
#define CARD_RENDER_SCALE(a) ((124.0f / a->GetWidth()) * g_tableRender.getTableScale())

// Size of a rendered card
#define CARD_RENDERED_SIZE(a) (a->GetWidth() * CARD_RENDER_SCALE(a))

class ClientCard
{
	public:
		ClientCard(uint8 suit, uint8 value, std::string texture);

		hgeSprite* Sprite() const;

		uint8 getSuit() const { return m_uiSuit; }
		uint8 getValue() const { return m_uiValue; }

	private:
		uint8 m_uiSuit;
		uint8 m_uiValue;

		std::string m_textureName;
};

#endif