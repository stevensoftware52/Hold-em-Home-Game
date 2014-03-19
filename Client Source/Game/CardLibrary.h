#ifndef _CARDLIBRARY_H
#define _CARDLIBRARY_H

#include "stdafx.h"

#define MAX_SUITS 4
#define MAX_VALUE 13

#define CARD_TEXTURE_NAMING "suit(%d) card(%d).png"

class CardLibrary
{
	public:
		CardLibrary();

		ClientCard* getCard(uint8 suit, uint8 value) const
		{
			assert(suit && value);

			if (suit <= MAX_SUITS && value <= MAX_VALUE)
				return m_cards[suit][value];

			return 0;
		}
				
	private:
		ClientCard* m_cards[MAX_SUITS + 1][MAX_VALUE + 1];
};

#endif