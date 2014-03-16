#include "stdafx.h"

// ------------------
// Constructor
Slider::Slider()
{
	m_sliderBar = new Button(SLIDER_BOUND_A, SLIDER_BOUND_B);
	m_minusButton = new Button(MINUS_B_BOUND_A, MINUS_B_BOUND_B);
	m_plusButton = new Button(PLUS_B_BOUND_A, PLUS_B_BOUND_B);
	m_slider = new Button(Vector2(), Vector2());

	m_bHolding = true;
	m_fLastMouseX = 0.0f;
}

// ------------------------
// Destructor
Slider::~Slider()
{
	delete m_sliderBar;
	delete m_minusButton;
	delete m_plusButton;
}

// --------
// Input
void Slider::Input()
{
	// Accept input from 

	float fMinValue = GetMinValue();
	float fMaxValue = GetMaxValue();

	if (m_minusButton->ButtonPressed())
		m_fCurrentNumber -= g_gameInfo.m_fBigBlind;
	else if (m_plusButton->ButtonPressed())
		m_fCurrentNumber += g_gameInfo.m_fBigBlind;
	else if (m_slider->ButtonPressed(true))
		m_bHolding = true;
	else if (m_sliderBar->ButtonPressed(true))
	{
		if (_input.x < m_sliderButtonPos.x)
			m_fCurrentNumber -= g_gameInfo.m_fBigBlind;

		if (_input.x > m_sliderButtonPos.x)
			m_fCurrentNumber += g_gameInfo.m_fBigBlind;
	}

	ConstrictNumber(m_fCurrentNumber);

	// Update input box text

	char str[24];
	sprintf_s(str, "%.2f", m_fCurrentNumber);
	g_tableRender.setMsg(str);
}

// ---------
// Render
void Slider::Render()
{	
	// If holding mouse on button thingy, dew the dew

	if (m_bHolding)
	{
		// If you move the slider at this rate once per frame it doesn't keep up. 
		// This is horseshit. I'm looping to compensate, do it properly some day you idiot.

		for (unsigned int i = 0; i < 50; ++i)
		{
			float diffX = (_input.x - m_sliderButtonPos.x) / 100.0f * (GetMaxValue() / 100.0f);
			m_fCurrentNumber += diffX;
			ConstrictNumber(m_fCurrentNumber);
		}

		// If they let go of the mouse
		m_bHolding = _hge->Input_GetKeyState(HGEK_LBUTTON);
	}

	// Slider bar

	if (hgeSprite* pSprite = g_hgeClient.sprite("slider_bar.png"))
	{
		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->Render(SLIDER_RENDER_X, SLIDER_RENDER_Y);
	}

	// Minus Button

	if (hgeSprite* pSprite = g_hgeClient.sprite("slider_minus.png"))
	{
		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->Render(SLIDER_MINUS_B_X, SLIDER_MINUS_B_Y);
	}

	// Plus Button

	if (hgeSprite* pSprite = g_hgeClient.sprite("slider_plus.png"))
	{
		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->Render(SLIDER_PLUS_B_X, SLIDER_PLUS_B_Y);
	}

	// Grabber thingy
	
	if (hgeSprite* pSprite = g_hgeClient.sprite("slider_grab.png"))
	{
		// Deduce where to render, it's not static
		//

		// Possible start, possible end
		Vector2 startRender(SLIDER_RENDER_X - (SLIDER_WIDTH / 2.0f) + (SLIDER_PM_BUTTON_SIZE / 1.5f), SLIDER_RENDER_Y);		
		Vector2 endRender(SLIDER_RENDER_X + (SLIDER_WIDTH / 2.0f) - (SLIDER_PM_BUTTON_SIZE / 1.5f), SLIDER_RENDER_Y);
		
		// Deduce an end point based on extruding a percentage from start to end

		float max_distance = Util::dist(startRender, endRender);
		float actual_distance = max_distance * ((m_fCurrentNumber - GetMinValue()) / (GetMaxValue() - GetMinValue()));

		m_sliderButtonPos = Util::extrude(startRender.x, startRender.y, endRender.x, endRender.y, actual_distance);

		// Render it

		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->Render(m_sliderButtonPos.x, m_sliderButtonPos.y);

		// Update button location
		
		delete m_slider;

		m_slider = new Button(
			Vector2(m_sliderButtonPos.x - 6.0f, m_sliderButtonPos.y - 12.0f), 
			Vector2(m_sliderButtonPos.x + 6.0f, m_sliderButtonPos.y + 12.0f));
	}
}

// ----------------------
// ConstrictNumber
void Slider::ConstrictNumber(float& fNumber)
{
	if (fNumber < GetMinValue())
		fNumber = GetMinValue();

	if (fNumber > GetMaxValue())
		fNumber = GetMaxValue();
}

// ----------------------
// ConstrictNumber
float Slider::GetMinValue() const
{
	// TODO: This
	return g_gameInfo.m_fBigBlind;
}

// ----------------------
// GetMaxValue
float Slider::GetMaxValue() const
{
	if (Player* pPlayer = g_tableRender.GetPlayer(g_gameInfo.m_uiGUID))
		return pPlayer->getMoney();

	return g_gameInfo.m_fBankroll;
}