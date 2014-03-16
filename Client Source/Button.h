#ifndef _BUTTON_H
#define _BUTTON_H

#include "stdafx.h"

// Size of a button when we're using the button sprite
#define BUTTON_SPRITE_SIZE 85.0f

// Formula to fit to scale, (a) being the sprite
#define BUTTON_SCALE(a) (BUTTON_SPRITE_SIZE / a->GetWidth())

class Button
{
	public:	
		Button(Vector2 renderPos, std::string title) :
			m_renderPos(renderPos), m_title(title) {}

		Button(Vector2 cornerA, Vector2 cornerB) :
			m_cornerA(cornerA), m_cornerB(cornerB) {}

		void Render();
		void setPosition(Vector2 pos) { m_renderPos = pos; }

		bool ButtonPressed() const;
		bool MousingOver() const;

	private:
		Vector2 m_cornerA;
		Vector2 m_cornerB;

		std::string m_title;

		Vector2 m_renderPos;
};

#endif