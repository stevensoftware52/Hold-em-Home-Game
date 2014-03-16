#ifndef _INPUTBOX_H
#define _INPUTBOX_H

#include "stdafx.h"

#define INPUTBOX_PULSE_INTERVAL 0.5f

enum
{
	STRICT_NONE,
	STRICT_NORMAL,	// Normal strictness allows letters, numbers, and the period/underscore character
	STRICT_MONEY	// Money strictness allows 1 period and numbers
};

class InputBox
{
	public:
		InputBox(float renderX, float renderY, DWORD align = HGETEXT_LEFT, float maxLength = 200.0f, std::string font = "tahoma13.fnt", uint8 bStrict = 0, DWORD color = 0xFFFFFFFF) :
			m_renderX(renderX), m_renderY(renderY), m_align(align), m_maxLength(maxLength), m_font(font), m_uiStrict(bStrict), m_color(color)
		{
			m_bPulse = false;
			m_pulseTimer = 0.0f;
		}

		void Input();
		void Render();
		void FocusLogic();

		void setMaxLength(float maxLength) { m_maxLength = maxLength; }
		
		bool InUse() const;
		bool BoxIsFull() const;

		virtual void PressedEnter() {}

		std::string m_typedMsg;

	protected:
		bool m_bPulse;

		uint8 m_uiStrict;

		float m_pulseTimer;
		float m_renderX;
		float m_renderY;
		float m_maxLength;

		DWORD m_align;
		DWORD m_color;

		std::string m_font;
};

#endif