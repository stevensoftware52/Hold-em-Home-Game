#include "stdafx.h"

// -----------------
// Constructor
ClientCard::ClientCard(uint8 suit, uint8 value, std::string texture)
{
	m_uiSuit = suit;
	m_uiValue = value;
	m_textureName = texture;
}

// -------------------
// Render
hgeSprite* ClientCard::Sprite() const
{
	return g_hgeClient.sprite(m_textureName);
}