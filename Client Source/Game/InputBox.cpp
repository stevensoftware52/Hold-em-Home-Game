#include "..\\stdafx.h"

// -----------------
// Input
void InputBox::Input()
{
	if (int character = _hge->Input_GetChar())
	{
		switch (character)
		{
			case HGEK_BACKSPACE:
				if (m_typedMsg.size())
					m_typedMsg.erase(m_typedMsg.end() - 1);
				break;

			case HGEK_ENTER:
				PressedEnter();
				break;

			case HGEK_ESCAPE:
				g_hgeClient.changeInputBox(0);
				break;

			// Inputting a character

			default:
				
				// Verify against strictness

				if (m_uiStrict == STRICT_NORMAL)
					if (!isalpha(character) && !isdigit(character) && character != '.' && character != '_')
						break;
				
				if (m_uiStrict == STRICT_MONEY)
				{
					if (character == '.')
					{
						if (find(m_typedMsg.begin(), m_typedMsg.end(), character) != m_typedMsg.end())
							break;
					}
					else if (!isdigit(character))
						break;
				}

				// Add if not full

				if (!BoxIsFull())
					m_typedMsg.push_back(character);

				break;
		}
	}

	m_pulseTimer += _hge->Timer_GetDelta();

	if (m_pulseTimer > INPUTBOX_PULSE_INTERVAL)
	{
		m_bPulse = !m_bPulse;
		m_pulseTimer = 0.0f;
	}
}

// -----------------
// Render
void InputBox::Render()
{
	std::string renderText = m_typedMsg;

	if (m_bPulse && InUse())
		renderText.push_back('|');

	if (hgeFont* pFont = g_hgeClient.font(m_font))
	{		
		pFont->SetColor(m_color);
		pFont->printf(Util::round(m_renderX), Util::round(m_renderY - (pFont->GetHeight() / 2.0f)), m_align, "%s", renderText.c_str());
		pFont->SetColor(0xFFFFFFFF);
	}
}

// ---------------------
// FocusLogic
//		Brings us into focus if clicked on
void InputBox::FocusLogic()
{
	if (hgeFont* pFont = g_hgeClient.font(m_font))
		if (Util::cordsInBox(_input.x, _input.y, m_renderX, m_renderY - pFont->GetHeight(), m_renderX + m_maxLength, m_renderY + pFont->GetHeight()))
			if (_hge->Input_GetKey() == HGEK_LBUTTON)
				g_hgeClient.changeInputBox(this);
}

// ---------------------
// BoxIsFull
bool InputBox::BoxIsFull() const
{
	if (!m_maxLength)
		return false;
	
	if (hgeFont* pFont = g_hgeClient.font(m_font))
		return pFont->GetStringWidth(m_typedMsg.c_str()) >= m_maxLength;

	return true;
}

// -----------------
// InUse
bool InputBox::InUse() const
{
	return g_hgeClient.curInputBox() == this;
}
