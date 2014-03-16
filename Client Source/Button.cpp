#include "stdafx.h"

// ------------------
// Render
void Button::Render()
{
	if (!m_title.size())
		return;

	std::string sprite = "button_idle.png";

	if (MousingOver() && _hge->Input_GetKeyState(HGEK_LBUTTON))
		sprite = "button_press.png";

	if (hgeSprite* pSprite = g_hgeClient.sprite(sprite.c_str()))
	{
		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->RenderEx(m_renderPos.x, m_renderPos.y, 0.0f, BUTTON_SCALE(pSprite), BUTTON_SCALE(pSprite));

		if (hgeFont* pFont = g_hgeClient.font("tahoma13b.fnt"))
			pFont->printf(m_renderPos.x, m_renderPos.y - (pFont->GetHeight() / 2.0f), HGETEXT_CENTER, m_title.c_str());
	}
}

// -----------------
// ButtonPressed
bool Button::ButtonPressed() const
{
	return _hge->Input_GetKey() == HGEK_LBUTTON && MousingOver();			
}

// ---------------------
// MousingOver
bool Button::MousingOver() const
{
	// Custom cords
	if (!m_title.size())
	{
		return Util::cordsInBox(_input.x, _input.y, m_cornerA.x, m_cornerA.y, m_cornerB.x, m_cornerB.y);
	}

	// Using the default sprite
	else
	{
		return Util::cordsInBox(_input.x, _input.y, 
			m_renderPos.x - (BUTTON_SPRITE_SIZE / 2.0f), 
			m_renderPos.y - (BUTTON_SPRITE_SIZE / 2.0f),
			m_renderPos.x + (BUTTON_SPRITE_SIZE / 2.0f), 
			m_renderPos.y + (BUTTON_SPRITE_SIZE / 2.0f));
	}

	return false;
}