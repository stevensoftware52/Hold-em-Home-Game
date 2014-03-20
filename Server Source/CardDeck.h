#ifndef _CARDDECK_H
#define _CARDDECK_H

#include "stdafx.h"

#define MAX_SUITS 4
#define MAX_VALUE 13

// The number of times we shuffle
#define SHUFFLE_DEPTH Util::randomNumber(50, 100)

struct Card
{
	Card() { uiSuit = 0, uiValue = 0; }
	Card(uint8 suit, uint8 value) : uiSuit(suit), uiValue(value) {}

	uint8 uiSuit;
	uint8 uiValue;
};

static std::string g_suitNames[MAX_SUITS + 1] =
{
	"ERROR",
	"h",
	"d",
	"c",
	"s"
};

static std::string g_valueNames[MAX_VALUE + 1] =
{
	"ERROR",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"T",
	"J",
	"Q",
	"K",
	"A"
};

enum eHandStrengths
{
	ONE_PAIR = 50000,
	TWO_PAIR = 100000,
	THREE_OF_KIND = 150000,
	STRAIGHT = 200000,
	FLUSH = 250000,
	FULL_HOUSE = 300000,
	FOUR_OF_KIND = 350000,
	STRAIGHT_FLUSH = 400000,
};

// Possible straights
#define MAX_POSSIBLE_STRAIGHTS 10
#define NUM_CARD_TO_STRAIGHT 5

static std::vector<uint8> g_possibleStraights[MAX_POSSIBLE_STRAIGHTS] =
{
	{ 13, 1, 2, 3, 4 },
	{ 1, 2, 3, 4, 5 },
	{ 2, 3, 4, 5, 6 },
	{ 3, 4, 5, 6, 7 },
	{ 4, 5, 6, 7, 8 },
	{ 5, 6, 7, 8, 9 },
	{ 6, 7, 8, 9, 10 },
	{ 7, 8, 9, 10, 11 },
	{ 8, 9, 10, 11, 12 },
	{ 9, 10, 11, 12, 13 }
};

class CardDeck
{
	public:
		CardDeck();

		void Shuffle();

		unsigned int CalculateHandStrength(std::vector<Card> cards, std::string& stringResult);

		Card DrawCard();		

		std::string GetHandStrToText(unsigned int strength);

		static std::string getCardName(Card card)
		{
			std::string result;

			if (card.uiSuit <= MAX_SUITS && card.uiValue <= MAX_VALUE)
			{
				result.append(g_valueNames[card.uiValue]);
				result.append(g_suitNames[card.uiSuit]);
			}

			assert(result.size());

			return result;
		};

	private:
		std::vector<Card> m_cardDeck;
};

#endif