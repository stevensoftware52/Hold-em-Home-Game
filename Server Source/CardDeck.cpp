#include "stdafx.h"

// ---------------
// Constructor
CardDeck::CardDeck()
{
	srand((unsigned int)time(NULL));
	
	Shuffle();

	/*while (true)
	{
		Shuffle();

		std::vector<Card> cards;

		for (unsigned int i = 0; i < 7; ++i)
			cards.push_back(DrawCard());

		std::string result;
		int handStr = CalculateHandStrength(cards, result);
	}

	system("pause");*/
}

// --------------
// Shuffle
void CardDeck::Shuffle()
{
	clock_t before = clock();

	unsigned int shuffleDepth = SHUFFLE_DEPTH;
	
	// Start with a default deck
	//

	std::vector<Card> vPossibleCards;

	for (uint8 s = 1; s <= MAX_SUITS; ++s)
		for (uint8 v = 1; v <= MAX_VALUE; ++v)
			vPossibleCards.push_back(Card(s, v));

	// Shuffle this many times
	//

	for (unsigned int i = 0; i < shuffleDepth; ++i)
	{
		// Make a copy of the deck
		std::vector<Card> vNewSeed = vPossibleCards;

		// Erase the deck
		vPossibleCards.clear();

		// Draw randomly from the copy, populating a new randomized deck
		while (vNewSeed.size())
		{
			// Remove a random card from vNewSeed and add it to vPossibleCards

			auto itr = vNewSeed.begin() + Util::randomNumber(0, vNewSeed.size());
			vPossibleCards.push_back(*itr);
			vNewSeed.erase(itr);
		}
	}
	
	// Store the generated deck into m_cardDeck
	//

	m_cardDeck.clear();
	m_cardDeck = vPossibleCards;

	// clock_t calcTime = clock() - before;
}

// -------------------
// DrawCard
Card CardDeck::DrawCard()
{
	Card result;

	if (m_cardDeck.size())
	{
		result = m_cardDeck[0];
		m_cardDeck.erase(m_cardDeck.begin());		
		return result;
	}

	printf("ERROR: Drew a blank card from an empty deck!!\n");
	return result;
}

// --------------------------
// CalculateHandStrength
unsigned int CardDeck::CalculateHandStrength(std::vector<Card> cards, std::string& stringResult)
{
	if (cards.size() < 5)
		return 0;

	char buffer[256];

	int uiResult = 7;

	// Variables
	//

		// Number we have of each suit, we can then look if any are >= 5
		std::vector<uint8> vNumOfEachSuit[MAX_SUITS + 1];

		// Number we have of each value, we can then look if any are > 1 and how many were > 1
		uint8 vNumOfEachValue[MAX_VALUE + 1];

		for (unsigned int i = 0; i < MAX_VALUE + 1; ++i)
			vNumOfEachValue[i] = 0;

		// A pointer to the highest straight we have, if left as 0 then we don't have one
		std::vector<uint8>* vHighestStraight = 0;

		// Will be filled with the suits of a straight if we have one
		std::vector<uint8> vStraightSuits;

		// Which pairs we have
		std::vector<uint8> vPairs;

		// Which three of kinds we have
		std::vector<uint8> vThreeOfKinds;

	// Okay, we know what we want, fill in the data
	//

		// Fill in suits and values
		for (unsigned int i = 0; i < cards.size(); ++i)
		{
			vNumOfEachSuit[cards[i].uiSuit].push_back(cards[i].uiValue);
			++vNumOfEachValue[cards[i].uiValue];
		}

		// Find highest possible straight
		for (int i = MAX_POSSIBLE_STRAIGHTS - 1; i >= 0 ; --i)
		{
			std::vector<uint8> uiMatches;

			for (uint8 c = 0; c < cards.size(); ++c)
			{
				// Don't consider duplicates when tallying how many matched
				if (find(uiMatches.begin(), uiMatches.end(), cards[c].uiValue) != uiMatches.end())
					continue;

				// If they have a card in this straight
				if (find(g_possibleStraights[i].begin(), g_possibleStraights[i].end(), cards[c].uiValue) != g_possibleStraights[i].end())
				{
					uiMatches.push_back(cards[c].uiValue);
					vStraightSuits.push_back(cards[c].uiSuit);
				}
			}

			// If we matched 5 cards to this straight, then its a straight
			if (uiMatches.size() == 5)
			{
				if (uiMatches.size() > 5)
					assert(0);

				vHighestStraight = &g_possibleStraights[i];
				break;
			}

			vStraightSuits.clear();
		}

	// Okay, we know what we have, see which is the biggest
	//

		// First, check if we have a straight that is a flush
		if (vHighestStraight)
		{
			// This should never happen, baka!
			if (vStraightSuits.size() < 5)
				assert(0);

			vStraightSuits.erase( Util::Unique( vStraightSuits.begin(), vStraightSuits.end() ), vStraightSuits.end() );

			// If a straight flush
			if (vStraightSuits.size() == 1)
			{
				sprintf_s(buffer, "Straight Flush, %s to %s", g_valueNames[(*vHighestStraight)[0]].c_str(), g_valueNames[(*vHighestStraight)[4]].c_str());
				return STRAIGHT_FLUSH + (*vHighestStraight)[4];
			}
			else
			{
				#define STRAIGHT_FORMULA STRAIGHT + (*vHighestStraight)[4]

				// See if this is the best hand
				uiResult = max(uiResult, STRAIGHT_FORMULA);

				// If it is the best hand
				if (uiResult == STRAIGHT_FORMULA)
					sprintf_s(buffer, "Straight, %s to %s", g_valueNames[(*vHighestStraight)[0]].c_str(), g_valueNames[(*vHighestStraight)[4]].c_str());
			}
		}

		// Check if we have a flush
		for (uint8 i = 1; i <= MAX_SUITS; ++i)
		{
			// If we have 5 of this suit
			if (vNumOfEachSuit[i].size() >= 5)
			{
				int flushValue = 0;
				std::vector<uint8> vValues;

				// Grab the five highest cards
				for (uint8 v = 0; v < 5; ++v)
				{
					auto itr = std::max_element(vNumOfEachSuit[i].begin(), vNumOfEachSuit[i].end());
					flushValue += *itr;
					vValues.push_back(*itr);
					vNumOfEachSuit[i].erase(itr);
				}

				#define FLUSH_FORMULA FLUSH + flushValue

				// See if this hand is best hand
				uiResult = max(uiResult, FLUSH_FORMULA);
				
				// If it is the best hand
				if (uiResult == FLUSH_FORMULA)								
					sprintf_s(buffer, "Flush, %s %s %s %s %s", 
						g_valueNames[vValues[0]].c_str(),
						g_valueNames[vValues[1]].c_str(),
						g_valueNames[vValues[2]].c_str(),
						g_valueNames[vValues[3]].c_str(),
						g_valueNames[vValues[4]].c_str());
			}
		}
	
		// Check if we have pairings
		for (uint8 i = 1; i <= MAX_VALUE; ++i)
		{
			// A pair
			if (vNumOfEachValue[i] == 2)
			{
				vPairs.push_back(i);

				#define ONE_PAIR_FORMULA ONE_PAIR + (15 * i)

				// See if this hand is best hand
				uiResult = max(uiResult, ONE_PAIR_FORMULA);

				// If this is the best hand
				if (uiResult == ONE_PAIR_FORMULA)
				{
					int kickersValue = 0;

					std::vector<uint8> finalKickers;
					std::vector<uint8> possibleKickers;

					// Deduce kickers
					for (uint8 c = 0; c < cards.size(); ++c)
						if (cards[c].uiValue != i)
							possibleKickers.push_back(cards[c].uiValue);
					
					// If we can't deduce 3 kickers, he doesn't have a 1 pair
					if (possibleKickers.size() >= 3)
					{
						// Grab best 3 kickers
						for (int v = 2; v >= 0; --v)
						{
							auto itr = std::max_element(possibleKickers.begin(), possibleKickers.end());
							kickersValue += *itr + (v * 15);
							finalKickers.push_back(*itr);
							possibleKickers.erase(itr);
						}

						// Increament hand score by value of card
						uiResult += kickersValue;

						sprintf_s(buffer, "Pair of %s's, kickers ", 
							g_valueNames[i].c_str());

						for (unsigned int i = 0; i < finalKickers.size(); ++i)
							strncat_s(buffer, g_valueNames[finalKickers[i]].c_str(), g_valueNames[finalKickers[i]].size());
					}
				}
			}
		
			// A three of kind
			if (vNumOfEachValue[i] == 3)
			{
				vThreeOfKinds.push_back(i);

				#define THREE_OF_KIND_FORMULA THREE_OF_KIND + (15 * i)

				// See if this hand is best hand
				uiResult = max(uiResult, THREE_OF_KIND_FORMULA);

				// If this is the best hand
				if (uiResult == THREE_OF_KIND_FORMULA)
				{
					int kickersValue = 0;

					std::vector<uint8> finalKickers;
					std::vector<uint8> possibleKickers;

					// Deduce a kicker
					for (uint8 c = 0; c < cards.size(); ++c)
						if (cards[c].uiValue != i)
							possibleKickers.push_back(cards[c].uiValue);

					// If we can't deduce 2 kickers, he doesn't have three of a kind
					if (possibleKickers.size() >= 2)
					{
						// Grab best 2 kickers
						for (int v = 1; v >= 0; --v)
						{
							auto itr = std::max_element(possibleKickers.begin(), possibleKickers.end());
							kickersValue += *itr + (v * 15);
							finalKickers.push_back(*itr);
							possibleKickers.erase(itr);
						}

						// Increament hand score by value of card
						uiResult += kickersValue;

						sprintf_s(buffer, "Three of a kind, %s's, kickers ", 
							g_valueNames[i].c_str());

						for (unsigned int i = 0; i < finalKickers.size(); ++i)
							strncat_s(buffer, g_valueNames[finalKickers[i]].c_str(), g_valueNames[finalKickers[i]].size());
					}
				}
			}
		
			// A four of kind
			if (vNumOfEachValue[i] == 4)
			{
				// See if this hand is best hand
				uiResult = max(uiResult, FOUR_OF_KIND + (15 * i));

				sprintf_s(buffer, "Four of a kind, %s's", 
					g_valueNames[i].c_str());
			}
		}

		// Check for two pairs
		if (vPairs.size() >= 2)
		{
			auto itrMax = std::max_element(vPairs.begin(), vPairs.end());
			auto itrMin = std::min_element(vPairs.begin(), vPairs.end());
		
			#define TWO_PAIR_FORMULA TWO_PAIR + (15 * (*itrMax)) + *itrMin

			// See if this hand is best hand
			uiResult = max(uiResult, TWO_PAIR_FORMULA);

			// If this is the best hand
			if (uiResult == TWO_PAIR_FORMULA)
			{
				std::vector<uint8> possibleKickers;

				// Deduce a kicker
				for (uint8 c = 0; c < cards.size(); ++c)
					if (find(vPairs.begin(), vPairs.end(), cards[c].uiValue) == vPairs.end())
						possibleKickers.push_back(cards[c].uiValue);

				// Kicker is largest card
				auto itr = std::max_element(possibleKickers.begin(), possibleKickers.end());

				// Increament hand score by value of card
				uiResult += *itr;

				sprintf_s(buffer, "%s's and %s's, %s kicker", 
					g_valueNames[vPairs[0]].c_str(),
					g_valueNames[vPairs[1]].c_str(),
					g_valueNames[*itr].c_str());
			}
		}

		// Check for full house
		if (vPairs.size() >= 1 && vThreeOfKinds.size() >= 1)
		{
			auto bestPair = std::max_element(vPairs.begin(), vPairs.end());
			auto bestThreeOfKind = std::max_element(vThreeOfKinds.begin(), vThreeOfKinds.end());

			int thisValue = FULL_HOUSE + (15 * (*bestThreeOfKind)) + (*bestPair);
		
			// See if this hand is best hand
			uiResult = max(uiResult, thisValue);
			
			// If this is the best hand
			if (uiResult == thisValue)
			{
				sprintf_s(buffer, "%s's full of %s's", 
						g_valueNames[*bestPair].c_str(),
						g_valueNames[*bestThreeOfKind].c_str());
			}
		}

		// Last but not least, check for a High Card
		//

		int highcardVal = 0;

		std::vector<uint8> vHighCards; // Will be filled highest to lowest
		std::vector<uint8> vValues;

		for (uint8 i = 0; i < cards.size(); ++i)
			vValues.push_back(cards[i].uiValue);

		// Get best 5 cards
		for (int v = 5; v >= 0; --v)
		{
			auto itr = std::max_element(vValues.begin(), vValues.end());
			highcardVal += *itr;
			vHighCards.push_back(*itr);
			vValues.erase(itr);
		}

		uiResult = max(uiResult, highcardVal);

		// If the best hand we have is the 5 highest cards out there
		if (uiResult == highcardVal)
			sprintf_s(buffer, "High cards %s%s%s%s%s", 
				g_valueNames[vHighCards[0]].c_str(),
				g_valueNames[vHighCards[1]].c_str(),
				g_valueNames[vHighCards[2]].c_str(),
				g_valueNames[vHighCards[3]].c_str(),
				g_valueNames[vHighCards[4]].c_str());

	// Return
	//

	stringResult = buffer;

	return uiResult;
}