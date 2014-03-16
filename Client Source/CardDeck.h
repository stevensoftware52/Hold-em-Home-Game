#ifndef _CARDDECK_H
#define _CARDDECK_H

#include "stdafx.h"

#define MAX_SUITS 4
#define MAX_VALUE 13

// The number of times we shuffle
#define SHUFFLE_DEPTH Util::randomNumber(250, 750)

struct Card
{
	Card() { uiSuit = 0, uiValue = 0; }
	Card(uint8 suit, uint8 value) : uiSuit(suit), uiValue(value) {}

	uint8 uiSuit;
	uint8 uiValue;
};

class CardDeck
{
	public:
		CardDeck();

		// Debug Functions
		//

		void Shuffle();

		Card DrawCard();		

	private:
		std::vector<Card> m_cardDeck;
};

#endif