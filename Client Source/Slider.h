#ifndef _SLIDER_H
#define _SLIDER_H

#include "stdafx.h"

// How wide the slider is
#define SLIDER_WIDTH 200.0f

// How tall the slider is
#define SLIDE_HEIGHT 20.0f

// Size of plus/minus button
#define SLIDER_PM_BUTTON_SIZE 35.0f

// Where the slider is rendered
#define SLIDER_RENDER_X (A_INPUTBOX_X)
#define SLIDER_RENDER_Y (A_INPUTBOX_Y - 40.0f)

// Press bounds for the slider
#define SLIDER_BOUND_A Vector2(A_INPUTBOX_X - (SLIDER_WIDTH / 2.0f) + (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_RENDER_Y - (SLIDE_HEIGHT / 2.0f))
#define SLIDER_BOUND_B Vector2(A_INPUTBOX_X + (SLIDER_WIDTH / 2.0f) - (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_RENDER_Y + (SLIDE_HEIGHT / 2.0f))

// Where the minus button goes
#define SLIDER_MINUS_B_X (A_INPUTBOX_X - (SLIDER_WIDTH / 2.0f))
#define SLIDER_MINUS_B_Y SLIDER_RENDER_Y

// Where the plus button goes
#define SLIDER_PLUS_B_X (A_INPUTBOX_X + (SLIDER_WIDTH / 2.0f))
#define SLIDER_PLUS_B_Y SLIDER_RENDER_Y

// Minus button bounds
#define MINUS_B_BOUND_A Vector2(SLIDER_MINUS_B_X - (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_MINUS_B_Y - (SLIDER_PM_BUTTON_SIZE / 2.0f))
#define MINUS_B_BOUND_B Vector2(SLIDER_MINUS_B_X + (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_MINUS_B_Y + (SLIDER_PM_BUTTON_SIZE / 2.0f))

// Plus button bounds
#define PLUS_B_BOUND_A Vector2(SLIDER_PLUS_B_X - (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_MINUS_B_Y - (SLIDER_PM_BUTTON_SIZE / 2.0f))
#define PLUS_B_BOUND_B Vector2(SLIDER_PLUS_B_X + (SLIDER_PM_BUTTON_SIZE / 2.0f), SLIDER_MINUS_B_Y + (SLIDER_PM_BUTTON_SIZE / 2.0f))

// The min/max value of the slider is based on what we see at the table, min being the min bet and max being our stack
//

class Slider
{
	public:
		Slider();
		~Slider();

		void Input();
		void Render();
		void SetNumber(float fNumber);
		void ConstrictNumber(float& fNumber);
		
		float GetMinValue() const;
		float GetMaxValue() const;

		float getNumber() const { return m_fCurrentNumber; }

	private:
		Button* m_slider;
		Button* m_sliderBar;
		Button* m_plusButton;
		Button* m_minusButton;

		bool m_bHolding;
		
		float m_fCurrentNumber;
		float m_fLastMouseX;

		Vector2 m_sliderButtonPos;
};

#endif