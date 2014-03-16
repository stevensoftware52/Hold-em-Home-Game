#include "stdafx.h"

// ---------------
// Constructor
CardDeck::CardDeck()
{
	srand((unsigned int)time(NULL));
	Shuffle();
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

	for (uint8 s = 0; s < MAX_SUITS; ++s)
		for (uint8 v = 0; v < MAX_VALUE; ++v)
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

/*

	CardDeck deck;

	Card players[10][2];

	for (unsigned int p = 0; p < 10; ++p)
		for (unsigned int c = 0; c < 2; ++c)
			players[p][c] = deck.DrawCard();

	for (unsigned int p = 0; p < 10; ++p)
	{
		std::string firstCard = CardLibrary::GetCardName(players[p][0].uiSuit, players[p][0].uiValue);
		std::string secondCard = CardLibrary::GetCardName(players[p][1].uiSuit, players[p][1].uiValue);

		printf("Player %d\n%s, %s\n\n", p, firstCard.c_str(), secondCard.c_str());
	}

*/