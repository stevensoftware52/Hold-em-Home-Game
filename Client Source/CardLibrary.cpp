#include "stdafx.h"

// ------------------
// Constructor
CardLibrary::CardLibrary()
{
	for (uint8 s = 1; s <= MAX_SUITS; ++s)
	{
		for (uint8 v = 1; v <= MAX_VALUE; ++v)
		{
			char dir[256];
			sprintf_s(dir, CARD_TEXTURE_NAMING, s, v);
			m_cards[s][v] = new ClientCard(s, v, dir); // LOAD TEXTURE
		}
	}
}