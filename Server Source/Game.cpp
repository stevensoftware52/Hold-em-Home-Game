#include "stdafx.h"

#define AI_GUID_MAX 6

// ----------------
// Game
Game::Game(PENNY bigBlind, PENNY smallBlind)
{
	ResetGame();
		
	m_penBigBlind = bigBlind;
	m_penSmallBlind = smallBlind;

	m_uiUniqueHandCount = 0;
	
	//AddPlayer("Computer1", 1, MAX_BUYIN);
	//AddPlayer("Computer2", 2, MAX_BUYIN);
	//AddPlayer("Computer3", 3, MAX_BUYIN);
}

// ----------
// Update
void Game::Update(unsigned int uiDiff)
{	
	PopulateAvailableDecisions();

	// If we don't have enough players for a game, then just keep everything at default settings
	if (!EnoughPlayersForGame())
	{
		ResetGame();
		return;
	}

	if (m_uiStageTimer >= uiDiff)
	{
		m_uiStageTimer -= uiDiff;
		return;
	}
	
	// We only reach this far if a game is in progress
	switch (m_uiCurrentStage)
	{
		// Hand hasn't begun, so deal cards (we'll say it takes 2 seconds to deal)
		case NOSTAGE:
		{
			BroadcastF("** HAND #%d STARTING **", ++m_uiUniqueHandCount);
			
			++m_uiCurrentStage;
			RotateBlinds();
			return;
		}
			
		// We don't call DealCards(); until the dealing finishes
		case NOSTAGE_DUMMY:
		{
			++m_uiCurrentStage;
			g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_DEAL));
			m_uiStageTimer = 250;
			break;
		}

		// Dealing in progress, wait for m_uiStageTimer to finish
		case DEALING:
		{
			DealCards();
			++m_uiCurrentStage;
			break;
		}

		// For these stages, we're simply waiting for them to make a decision via a packet
		// If their time runs out we just call PlayerFolds and then NextSeatToAct
		case PREFLOP:
		case FLOP:
		case TURN:
		case RIVER:
		{
			if (m_bSkipToShowdown || NumPlayersWithStack(true, true) < 1)
				Progress();
			else
			{
				// If decision time runs out, move to time bank
				if (m_seats[m_uiSeatToAct].pPlayer->dDecisionTime <= uiDiff)
				{
					m_seats[m_uiSeatToAct].pPlayer->dDecisionTime = 0;

					// If time bank runs out, they're forced to fold
					if (m_seats[m_uiSeatToAct].pPlayer->dTimeBank <= uiDiff)
					{
						PlayerFolds(m_seats[m_uiSeatToAct].pPlayer);
						Progress();
					}
					else
						m_seats[m_uiSeatToAct].pPlayer->dTimeBank -= (DWORD)uiDiff;
				}
				else
				{
					m_seats[m_uiSeatToAct].pPlayer->dDecisionTime -= (DWORD)uiDiff;
					
					// Time JUST ran out
					if (m_seats[m_uiSeatToAct].pPlayer->dDecisionTime <= uiDiff)
					{		
						if (Session* pSession = g_gameFloor.GetSession(m_seats[m_uiSeatToAct].pPlayer->accountId))
							pSession->SendMessageTo(PackOpcode(m_gameBuffer, OPCODE_HAND_TIMEBANK), m_gameBuffer);

						// Not let repeat
						m_seats[m_uiSeatToAct].pPlayer->dDecisionTime = 0;
					}
				}

				// DEBUG: Have AI check/call

				if (m_seats[m_uiSeatToAct].pPlayer->accountId <= AI_GUID_MAX)
				{
					if (m_uiCurrentStage > PREFLOP)
						ProcessPlayerDecision(m_seats[m_uiSeatToAct].pPlayer->accountId, "All In");
					else
						ProcessPlayerDecision(m_seats[m_uiSeatToAct].pPlayer->accountId, "Check");
						
					ProcessPlayerDecision(m_seats[m_uiSeatToAct].pPlayer->accountId, "Call");
				}
			}

			break;
		}

		// Wait for the timer to end, then restart
		case SHOWDOWN:
		case HANDISOVER:
			if (m_uiStageTimer <= uiDiff)
				EndHand();
			break;
	}
}

// ---------------------
// AddPlayer
bool Game::AddPlayer(std::string name, unsigned int accountId, PENNY stack)
{
	if (NumPlayers() >= NUM_SEATS)
		return false;

	if (getPlayer(accountId))
		return false;

	Player* pPlayer = new Player(name, accountId, stack);
		
	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (!m_seats[i].pPlayer)
		{
			m_seats[i].pPlayer = pPlayer;
			BroadcastF("%s sitting down in seat %d with $%.2f", pPlayer->name.c_str(), i, float(pPlayer->penStack) / 100.0f);
			g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_NEWPLAYER));
			return true;
		}
	}

	delete pPlayer;
	printf("ERROR: NumPlayers() < NUM_SEATS but failed to add player.\n");
	return false;
}

// --------------------------
// HavePlayersForGame
uint8 Game::NumPlayers(bool bPostedPlayers) const
{
	uint8 numPlayers = 0;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && (!bPostedPlayers || m_seats[i].pPlayer->bPostedBB))
		{
			// You're not a posted player if your stack is empty
			if (bPostedPlayers && m_seats[i].pPlayer->penStack < 1)
				continue;

			++numPlayers;
		}
	}

	return numPlayers;
}

// --------------------------
// NumPlayersWithHolding
uint8 Game::NumPlayersWithHolding() const
{
	uint8 numPlayers = 0;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer && playerHasHolding(m_seats[i].pPlayer))
			++numPlayers;

	return numPlayers;
}

// ---------------
// EndHand
void Game::EndHand()
{
	// Game resets if only 1 person left with hand
	if (NumPlayersWithStack(true, true) < 1)
	{
		ResetGame();
		return;
	}

	m_board.clear();

	m_uiSeatToAct = 0;
	m_uiCurrentStage = NOSTAGE;
	m_uiStageTimer = 0;
	m_penPot = 0;
	m_bSkipToShowdown = false;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer)
		{
			// Remove if offline

			Session* pSession = g_gameFloor.GetSession(m_seats[i].pPlayer->accountId);

			if (!pSession && m_seats[i].pPlayer->accountId > AI_GUID_MAX)
			{
				delete m_seats[i].pPlayer;
				m_seats[i].pPlayer = 0;
				continue;
			}

			// Affect bankroll by this much

			if (m_seats[i].pPlayer->penStack != m_seats[i].pPlayer->penStackBeforeHand)
			{
				int diff = m_seats[i].pPlayer->penStack - m_seats[i].pPlayer->penStackBeforeHand;

				if (diff)
					AffectBankroll(m_seats[i].pPlayer, diff);
			}

			m_seats[i].pPlayer->penStackBeforeHand = 0;
			m_seats[i].pPlayer->penBet = 0;			
			m_seats[i].pPlayer->cards[0].uiSuit = 0;
			m_seats[i].pPlayer->cards[0].uiValue = 0;				
			m_seats[i].pPlayer->cards[1].uiSuit = 0;
			m_seats[i].pPlayer->cards[1].uiValue = 0;
			m_seats[i].pPlayer->dDecisionTime = MAX_DECISION_TIME;
			m_seats[i].pPlayer->bNeedToAct = false;

			// Increase time bank at the end of every hand

			m_seats[i].pPlayer->dTimeBank += PLAYER_TIMEBANK_DEFAULT;	

			if (m_seats[i].pPlayer->dTimeBank >= PLAYER_MAX_TIMEBANK)
				m_seats[i].pPlayer->dTimeBank = PLAYER_MAX_TIMEBANK;

			// Player is no longer posted if has no moneys

			if (m_seats[i].pPlayer->penStack < 1)
				m_seats[i].pPlayer->bPostedBB = false;
		}
	}

	// Tell clients the hand is over
	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_OVER));
}

// ------------
// ResetGame
//		Puts everything to default starting numbers
void Game::ResetGame()
{
	m_board.clear();

	m_bFirstBigBlind = true;
	m_uiSeatToAct = 0;
	m_uiDealerSeat = 0;
	m_uiSmallBlindSeat = 1;
	m_uiBigBlindSeat = 2;
	m_uiCurrentStage = NOSTAGE;
	m_uiStageTimer = 0;
	m_penPot = 0;
	m_uiStageTimer = 5000;
	m_bSkipToShowdown = false;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer)
		{
			m_seats[i].pPlayer->penStackBeforeHand = 0;
			m_seats[i].pPlayer->penBet = 0;
			m_seats[i].pPlayer->bPostedBB = false;
			m_seats[i].pPlayer->dTimeBank = PLAYER_TIMEBANK_DEFAULT;	
			m_seats[i].pPlayer->dDecisionTime = MAX_DECISION_TIME;			
			m_seats[i].pPlayer->cards[0].uiSuit = 0;
			m_seats[i].pPlayer->cards[0].uiValue = 0;				
			m_seats[i].pPlayer->cards[1].uiSuit = 0;
			m_seats[i].pPlayer->cards[1].uiValue = 0;

			// Remove if offline

			Session* pSession = g_gameFloor.GetSession(m_seats[i].pPlayer->accountId);

			if (!pSession && m_seats[i].pPlayer->accountId > AI_GUID_MAX)
			{
				delete m_seats[i].pPlayer;
				m_seats[i].pPlayer = 0;
			}
		}
	}
}

// ------------------------
// DealCards
void Game::DealCards()
{
	m_cardDeck.Shuffle();

	// Deal server side

	for (uint8 c = 0; c < NUM_HOLD_CARDS; ++c)
	{		
		for (int i = m_uiDealerSeat + 1, count = 0; count < NUM_SEATS; ++i, ++count)
		{
			NormalizeSeatNumber(i);

			// If they're not in the hand, skip
			if (!m_seats[i].pPlayer || !m_seats[i].pPlayer->penStackBeforeHand)
				continue;

			if (m_seats[i].pPlayer->penStack >= 1 && (m_seats[i].pPlayer->bPostedBB || m_bFirstBigBlind))
			{
				Card card = m_cardDeck.DrawCard();

				m_seats[i].pPlayer->cards[c] = card;		
				m_seats[i].pPlayer->bNeedToAct = true;
			}
		}
	}

	// Tell clients about their cards
		
	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{				
		if (!m_seats[i].pPlayer || !playerHasHolding(m_seats[i].pPlayer))
			continue;

		if (Session* pSession = g_gameFloor.GetSession(m_seats[i].pPlayer->accountId))
		{
			std::vector<Card> cards;
			cards.push_back(m_seats[i].pPlayer->cards[0]);
			cards.push_back(m_seats[i].pPlayer->cards[1]);

			pSession->SendMessageTo(Session::BuildPlayerCardInfo(m_gameBuffer, m_seats[i].pPlayer->accountId, cards), m_gameBuffer);
		}
	}

	// First to act after dealing cards is player after big blind

	m_uiSeatToAct = m_uiBigBlindSeat;
	NextSeatToAct();

	m_bFirstBigBlind = false;
}

// -------------------------
// PlayerFolds
void Game::PlayerFolds(Player* pPlayer)
{
	BroadcastF("Player %s folds.", pPlayer->name.c_str());
	
	pPlayer->bNeedToAct = false;
	pPlayer->cards[0].uiSuit = 0;
	pPlayer->cards[0].uiValue = 0;				
	pPlayer->cards[1].uiSuit = 0;
	pPlayer->cards[1].uiValue = 0;	

	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_MUCK));

	CheckHandOver();

	g_gameFloor.BroadcastPacket(m_gameBuffer, Session::BuildPlayerCardInfo(m_gameBuffer, pPlayer->accountId, std::vector<Card>()));
}
 
// ----------------------
// CheckHandOver
void Game::CheckHandOver()
{
	if (NumPlayersWithHolding() <= 1)
	{
		AwardWinners();
	}
}

// ----------------------
// RemovePlayer
void Game::RemovePlayer(unsigned int accountId)
{
	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->accountId == accountId)
		{
			if (playerHasHolding(m_seats[i].pPlayer) || m_seats[i].pPlayer->penBet)
				return;

			delete m_seats[i].pPlayer;
			m_seats[i].pPlayer = 0;
		}
	}
}

// ---------------------
// RefreshNeedToAct
void Game::RefreshNeedToAct(Player* pExcept)
{
	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && m_seats[i].pPlayer != pExcept)
		{
			// Can't act if all in
			if (m_seats[i].pPlayer->penBet >= m_seats[i].pPlayer->penStack)
			{
				m_seats[i].pPlayer->bNeedToAct = false;
			}

			// Can't act if not having a holding
			else if (!playerHasHolding(m_seats[i].pPlayer))
			{
				m_seats[i].pPlayer->bNeedToAct = false;
			}
			
			// Can act
			else
			{
				m_seats[i].pPlayer->dDecisionTime = MAX_DECISION_TIME;
				m_seats[i].pPlayer->bNeedToAct = true;
			}
		}
	}
}

// --------------------
// AwardWinners
void Game::AwardWinners()
{
	std::vector<Player*> vWinners;

	// Scoop up any folded bets

	PENNY totalBetsOutThere = 0;
	
	ScoopUpBets();

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		// Anyone with a holding is a potential winner
		if (m_seats[i].pPlayer && playerHasHolding(m_seats[i].pPlayer))
			vWinners.push_back(m_seats[i].pPlayer);
	}
	
	m_penPot += totalBetsOutThere;

	m_uiStageTimer = 5000;
	m_uiCurrentStage = HANDISOVER;

	// If only 1 player with holding, he won by default

	if (NumPlayersWithHolding() > 1)
	{
		vWinners.clear();

		int bestHand = 0;

		// Calculate the best hand during first loop, and during the second loop anyone with that hand is a winner

		for (uint8 r = 0; r < 2; ++r)
		{
			for (uint8 i = 0; i < NUM_SEATS; ++i)
			{
				if (!m_seats[i].pPlayer)
					continue;

				std::vector<Card> sevenCards = m_board;
			
				if (playerHasHolding(m_seats[i].pPlayer))
				{
					sevenCards.push_back(m_seats[i].pPlayer->cards[0]);
					sevenCards.push_back(m_seats[i].pPlayer->cards[1]);

					std::string stringResult;
					int handStrength = m_cardDeck.CalculateHandStrength(sevenCards, stringResult);
					
					// During the second loop anyone with that hand is a winner
					if (r > 0)
					{
						if (handStrength >= bestHand)
						{
							BroadcastF("%s shows %s.", m_seats[i].pPlayer->name.c_str(), stringResult.c_str());
							BroadcastPlayerHand(m_seats[i].pPlayer);
							vWinners.push_back(m_seats[i].pPlayer);
						}
						else
						{
							BroadcastF("%s mucks [%s] [%s]", 
								m_seats[i].pPlayer->name.c_str(), 
								m_cardDeck.getCardName(m_seats[i].pPlayer->cards[0]).c_str(),
								m_cardDeck.getCardName(m_seats[i].pPlayer->cards[1]).c_str());
							
							g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_MUCK));
						}
					}
					else
					{
						if (handStrength > bestHand)
							bestHand = handStrength;
					}
				}
			}
		}
	}

	AwardPot(vWinners);
}

// ------------------------
// AwardPot
void Game::AwardPot(std::vector<Player*> vWinners)
{
	if (!vWinners.size())
		return;

	//if (vWinners.size() > 1)
	//	system("pause");
	
	// The biggest peice of the pie you can take
	
	PENNY potInPennies = m_penPot;
	PENNY iMostAnyoneExtract = potInPennies / vWinners.size();

	for (unsigned int i = 0; i < vWinners.size(); ++i)
	{
		// The most you can win is based on your stack pre-deal and what other people put in

		int iMostICanWin = vWinners[i]->penStackBeforeHand;
		
		for (unsigned int o = 0; o < NUM_SEATS; ++o)
		{
			if (vWinners[i] == m_seats[o].pPlayer || !m_seats[o].pPlayer || m_seats[o].pPlayer->penStackBeforeHand < 1)
				continue;

			iMostICanWin += min(vWinners[i]->penStackBeforeHand, m_seats[o].pPlayer->penStackBeforeHand);
		}

		// The amount to extract is...

		int iAmountToExtract = 0;

		if (iMostICanWin > iMostAnyoneExtract)
			iAmountToExtract = iMostAnyoneExtract;
		else
			iAmountToExtract = iMostICanWin;
		
		// Extract it

		potInPennies -= iAmountToExtract;

		vWinners[i]->penStack += iAmountToExtract;

		if (iAmountToExtract)
		{
			BroadcastF("%s collected $%.2f from pot", vWinners[i]->name.c_str(), PENNY_F(iAmountToExtract));			
			g_gameFloor.BroadcastPacket(m_gameBuffer, Session::BuildPotWinner(m_gameBuffer, vWinners[i]->accountId, iAmountToExtract));
		}
	}

	m_penPot = potInPennies;

	if (m_penPot && m_penPot < 1)
		assert(0);
		
	// If there's money left over, repeat process but remove hands from winners

	if (m_penPot >= 1)
	{
		for (uint8 i = 0; i < vWinners.size(); ++i)
		{
			vWinners[i]->cards[0].uiSuit = 0;
			vWinners[i]->cards[0].uiValue = 0;
			vWinners[i]->cards[1].uiSuit = 0;
			vWinners[i]->cards[1].uiValue = 0;
		}

		AwardWinners();
	}
}

// ----------------------
// NextSeatToAct
void Game::NextSeatToAct()
{
	if (!someoneNeedsToAct() || NumPlayersWithHolding() <= 1 || NumPlayersWithStack(true, true) < 1)
		return;	
	
	// Loop until find person to act next

	int numLoop = 0;
	int nextSeatToAct = m_uiSeatToAct;

	while (true)
	{
		++nextSeatToAct;
		++numLoop;
		NormalizeSeatNumber(nextSeatToAct);

		if (
			m_seats[nextSeatToAct].pPlayer &&						// If there's a player in this seat
			playerHasHolding(m_seats[nextSeatToAct].pPlayer) &&		// If this player has a holding
			m_seats[nextSeatToAct].pPlayer->penStack >= 1			// If they have a stack left
			)

		{
			if (m_seats[nextSeatToAct].pPlayer->bNeedToAct)
				break;
		}

		if (numLoop > NUM_SEATS * 2)
		{
			printf("ERROR: NextSeatToAct failed, breaking");
			break;
		}
	}

	m_uiSeatToAct = nextSeatToAct;

	BroadcastF("%s, it's your turn to act.", m_seats[m_uiSeatToAct].pPlayer->name.c_str());
}

// -----------------------
// LargestBetOutThere
PENNY Game::LargestBetOutThere()
{
	PENNY max_bet = 0;

	for (unsigned int i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && playerHasHolding(m_seats[i].pPlayer))
			if (m_seats[i].pPlayer->penBet > max_bet)
				max_bet = m_seats[i].pPlayer->penBet;
	}

	return max_bet;
}

// --------------------
// ScoopUpBets
void Game::ScoopUpBets()
{
	for (unsigned int i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->penBet >= 1)
		{			
			m_penPot += m_seats[i].pPlayer->penBet;
			m_seats[i].pPlayer->penBet = 0;
		}
	}
}

// ----------------------
// RotateBlinds
void Game::RotateBlinds()
{
	if (!EnoughPlayersForGame())
	{
		printf("CRASH: RotateBlinds() called but not enough players for game.\n");
		assert(0);
	}
	
	// Write down everyones stack before the hand began
	//
	
	for (uint8 i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer)
			m_seats[i].pPlayer->penStackBeforeHand = m_seats[i].pPlayer->penStack;		

	// If this is the first hand of the table then everyone is posted
	//

	if (m_bFirstBigBlind)
	{
		for (uint8 i = 0; i < NUM_SEATS; ++i)
		{
			if (m_seats[i].pPlayer && m_seats[i].pPlayer->penStack >= m_penBigBlind)
				m_seats[i].pPlayer->bPostedBB = true;
		}
	}

	// Loop until found valid player to post big blind
	//

	bool bLooping = true;

	while (bLooping)
	{
		// Roate blind
		++m_uiBigBlindSeat;

		// Seat 5 -> Seat 0
		if (m_uiBigBlindSeat >= NUM_SEATS)
			m_uiBigBlindSeat = 0;

		for (uint8 i = 0; i < NUM_SEATS; ++i)
		{
			if (
					m_seats[i].pPlayer && 							// If there's a player at this seat			
					m_seats[i].pPlayer->penStack >= m_penBigBlind	// If he has enough for the big blind
					&& i == m_uiBigBlindSeat						// If this is the big blind seat we're testing
				)						
			{
				BroadcastF("%s posts the big blind.", m_seats[i].pPlayer->name.c_str());

				m_seats[i].pPlayer->bPostedBB = true;								
				m_seats[i].pPlayer->penBet += ExtractBet(m_seats[i].pPlayer, m_penBigBlind);

				bLooping = false;
				break;
			}
		}
	}

	// Loop until found valid player to post small blind
	//

	int max_loops = 0;

	for (int i = (int)m_uiBigBlindSeat - 1; true; --i)
	{		
		int thisSeat = i;

		++max_loops;

		if (max_loops > NUM_SEATS)
		{
			ResetGame();
			return;
		}

		NormalizeSeatNumber(thisSeat);
		
		if (
				m_seats[thisSeat].pPlayer &&								// If there's a player at this seat
				m_seats[thisSeat].pPlayer->penStack >= m_penSmallBlind &&	// If he has enough for the SMALL blind
				thisSeat != m_uiBigBlindSeat &&								// If this is NOT the BIG blind seat
				(m_seats[thisSeat].pPlayer->bPostedBB || m_bFirstBigBlind)	// You can't post small blind without having posted big blind, unless this is the first small blind
			)		
		{
			m_uiSmallBlindSeat = thisSeat;
			m_seats[thisSeat].pPlayer->penBet += ExtractBet(m_seats[thisSeat].pPlayer, m_penSmallBlind);

			BroadcastF("%s posts the small blind.", m_seats[thisSeat].pPlayer->name.c_str());
			break;
		}
	}

	// Loop until found valid player for dealer
	//
	
	for (int i = (int)m_uiSmallBlindSeat - 1; true; --i)
	{		
		if (NumPlayers(true) == 2)
		{
			m_uiDealerSeat = m_uiSmallBlindSeat;
			break;
		}

		int thisSeat = i;

		NormalizeSeatNumber(thisSeat);

		// You can't be the dealer if you haven't posted
		if (m_seats[thisSeat].pPlayer && m_seats[thisSeat].pPlayer->bPostedBB)
		{
			m_uiDealerSeat = thisSeat;
			BroadcastF("%s is the dealer.", m_seats[thisSeat].pPlayer->name.c_str());
			break;
		}
	}

	// Loop until decided who's "after" big blind (they are next, or first, to act)
	//
	
	int loops = 0;

	while (true)
	{
		int seatToAct = (int)m_uiBigBlindSeat + 1 + loops;
		NormalizeSeatNumber(seatToAct);
		m_uiSeatToAct = seatToAct;

		if (m_seats[m_uiSeatToAct].pPlayer)
			break;

		++loops;
	}

	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_CHIP));
}

// --------------------
// ExtractBet
PENNY Game::ExtractBet(Player* pPlayer, PENNY penAmount) const
{
	if (penAmount <= 0)
		return 0;
	
	PENNY penAmountExtracted = 0;

	while (pPlayer->penStack && penAmountExtracted < penAmount)
	{
		--pPlayer->penStack;
		++penAmountExtracted;
	}

	return penAmountExtracted;
}

// ----------------------
// EnoughPlayersForGame
bool Game::EnoughPlayersForGame() const
{
	if (handInProgress())
		return true;
	
	return NumPlayersWithStack() >= 2;
}

// ----------------------
// NumPlayersWithStack
uint8 Game::NumPlayersWithStack(bool bDealtPlayersOnly, bool bHoldingOnly) const
{
	uint8 numPlayers = 0;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->penStack >= 1)
		{
			if (bDealtPlayersOnly && !m_seats[i].pPlayer->penStackBeforeHand)
				continue;

			if (bHoldingOnly && !playerHasHolding(m_seats[i].pPlayer))
				continue;

			++numPlayers;
		}
	}

	return numPlayers;
}

void Game::NormalizeSeatNumber(int& seat)
{
	while (true)
	{
		// Convert a negative i to a seat
		if (seat < 0)
		{
			seat = NUM_SEATS + seat;
			continue;
		}

		// After seat 5 is seat 0, if its seat 7 that's actually seat 1
		if (seat >= NUM_SEATS)
		{
			seat -= NUM_SEATS;
			continue;
		}

		break;
	}
}

// ------------------------
// BroadcastChatMsg
void Game::BroadcastF(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(m_gameBuffer, fmt, args);
	va_end(args);

	std::string msg = m_gameBuffer;

	g_gameFloor.BroadcastPacket(m_gameBuffer, Session::BuildChatMsg(m_gameBuffer, msg, CHATTYPE_SERVER_MSG));
}

// --------
// SendF
void Game::SendF(unsigned int accountId, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(m_gameBuffer, fmt, args);
	va_end(args);

	std::string msg = m_gameBuffer;

	if (Session* pSession = g_gameFloor.GetSession(accountId))
		pSession->SendMessageTo(Session::BuildChatMsg(m_gameBuffer, msg, CHATTYPE_DIRECT_SERVER_MSG), m_gameBuffer);
}

// ------------------------
// PlayerAtTable
bool Game::PlayerAtTable(unsigned int accountId) const
{
	for (unsigned int i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->accountId == accountId)
			return true;

	return false;
}

// ------------------------ 
// SomeoneNeedsToAct
uint8 Game::NumNeedToAct() const
{
	uint8 uiNumNeedToAct = 0;

	for (unsigned int i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->bNeedToAct)
			++uiNumNeedToAct;

	return uiNumNeedToAct;
}

// ------------------------
// GetAvailableDecisions
std::vector<std::string> Game::GetAvailableDecisions(unsigned int accountId) const
{
	for (unsigned int i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->accountId == accountId)
			return m_seats[i].pPlayer->vDecisionChoices;

	std::vector<std::string> result;
	result.push_back("Sit Down");
	return result;
}

// ----------------------------
// PopulateAvailableDecisions
void Game::PopulateAvailableDecisions()
{
	for (unsigned int i = 0; i < NUM_SEATS; ++i)
		if (m_seats[i].pPlayer)
			m_seats[i].pPlayer->vDecisionChoices = DeducePossibleDecisions(m_seats[i].pPlayer);
}

// ----------------------------
// DeducePossibleDecisions
std::vector<std::string> Game::DeducePossibleDecisions(Player* pPlayer)
{
	PENNY largestPetOutThere = LargestBetOutThere();

	std::vector<std::string> result;

	// Always the choice to leave
	result.push_back("Sit Out");

	if (!EnoughPlayersForGame() || m_uiCurrentStage >= SHOWDOWN || m_uiCurrentStage < PREFLOP)
		return result;

	// If it's not their turn to act
	if (m_seats[m_uiSeatToAct].pPlayer != pPlayer || m_bSkipToShowdown)
		return result;

	// If they don't have a holding
	if (!playerHasHolding(pPlayer))
		return result;

	// If they're all in
	if (!pPlayer->penStack)
		return result;

	// Fold is always a choice
	result.push_back("Fold");
		
	// If the largest bet out there is equal to mine
	if (largestPetOutThere == pPlayer->penBet)
		result.push_back("Check");

	// If the largest bet out there is larger than mine, we can Call or Bet/Raise
	if (largestPetOutThere > pPlayer->penBet)
	{
		result.push_back("Call");

		if (pPlayer->penStack > largestPetOutThere && NumPlayersWithStack(true, true) > 1)
		{
			result.push_back("Bet/Raise");
			result.push_back("All In");
		}
	}

	// If we're the big blind then we can Bet/Raise or All In
	if (m_seats[m_uiBigBlindSeat].pPlayer == pPlayer || !largestPetOutThere)
	{
		result.push_back("Bet/Raise");
		result.push_back("All In");
	}

	return result;
}

// -----------------------
// ProcessPlayerDecision
void Game::ProcessPlayerDecision(unsigned int accountId, std::string decision, float ffAmount)
{
	// Money can only have two decimal points
	//
	
	PENNY penAmount = PENNY(ffAmount * 100.0f);

	// Find player
	//

	uint8 seat = 0;
	Player* pPlayer = 0;

	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (m_seats[i].pPlayer && m_seats[i].pPlayer->accountId == accountId)
		{
			pPlayer = m_seats[i].pPlayer;
			seat = i;
			break;
		}
	}

	if (!pPlayer)
	{
		// Can only sit down if not already at table
		if (decision == "Sit Down")
		{
			if (Session* pSession = g_gameFloor.GetSession(accountId))
			{
				if (!penAmount)
				{
					SendF(accountId, "Please specify how much to buy in for.");
					return;
				}
				
				// Not let sit down with more than they have
				if (penAmount > pSession->getBankrollP())
				{
					SendF(accountId, "You don't have $%.2f. Your bankroll is $%.2f.", PENNY_F(penAmount), PENNY_F(pSession->getBankrollP()));
					return;
				}

				// Not let sit down without enough
				if (penAmount < MIN_BUYIN)
				{
					SendF(accountId, "The minimum buy-in is $%.2f", float(MIN_BUYIN) / 100.0f);
					return;
				}

				// Not let sit down with too much
				if (penAmount > MAX_BUYIN)
				{
					SendF(accountId, "The maximum buy-in is $%.2f", float(MAX_BUYIN) / 100.0f);
					return;
				}

				AddPlayer(pSession->getName(), accountId, penAmount);
			}
			else
				; // assert(0);
		}
		else
			printf("ERROR: Player not at table but asked to make decision, accountId %d decision %s\n", accountId, decision.c_str());

		return;
	}

	// Validate decision
	//

	if (decision == "Sit Out")
	{
		if (playerHasHolding(pPlayer) || pPlayer->penBet)
			SendF(accountId, "You can't sit out while you're in a hand.");
		else
			RemovePlayer(pPlayer->accountId);

		return;
	}

	if (find(pPlayer->vDecisionChoices.begin(), pPlayer->vDecisionChoices.end(), decision) == pPlayer->vDecisionChoices.end())
	{
		printf("ERROR: Account %d asked to make invalid decision %s\n", accountId, decision.c_str());
		return;
	}

	if (m_uiSeatToAct != seat)
	{
		printf("ERROR: Player account %d tried to act out of turn\n", accountId);
		return;
	}

	// Process decision
	//
	
	if (decision == "Fold")
	{
		pPlayer->bNeedToAct = false;
		PlayerFolds(pPlayer);
		Progress();
	}

	if (decision == "Check")
	{
		if (LargestBetOutThere() > pPlayer->penBet)
		{
			SendF(accountId, "You can't check, there's $%.2f left to call.", float(LargestBetOutThere() - pPlayer->penBet) / 100.0f);
			return;
		}
		
		BroadcastF("%s checks.", pPlayer->name.c_str());

		g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_CHECK));

		pPlayer->bNeedToAct = false;
		Progress();
	}

	if (decision == "Call")
	{
		PENNY penCallAmount = LargestBetOutThere() - pPlayer->penBet;

		assert(penCallAmount >= 1);

		penCallAmount = ExtractBet(pPlayer, penCallAmount);

		BroadcastF("%s calls $%.2f.", pPlayer->name.c_str(), float(penCallAmount) / 100.0f);
		g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_CHIP));
		
		pPlayer->bNeedToAct = false;
		pPlayer->penBet += penCallAmount;
		Progress();
	}

	if (decision == "Bet/Raise")
	{
		penAmount -= pPlayer->penBet;

		//if (!penAmount)
		//{
		//	SendF(accountId, "Please enter an amount.");
		//	return;
		//}

		if (penAmount > pPlayer->penStack)
		{
			SendF(accountId, "$%.2f is too much, you only have $%.2f in your stack.", PENNY_F(penAmount), PENNY_F(pPlayer->penStack));
			return;
		}

		if (penAmount + pPlayer->penBet < LargestBetOutThere() * 2)
		{
			SendF(accountId, "The minimum is $%.2f.", PENNY_F(LargestBetOutThere() + (LargestBetOutThere() - pPlayer->penBet)));
			return;
		}
				
		if (ExtractBet(pPlayer, penAmount) != penAmount)
		{
			printf("CRASH: Corrupt Bet/Raise from player\n");
			assert(0);
		}
		
		BroadcastF("%s bet/raise $%.2f.", pPlayer->name.c_str(), PENNY_F(penAmount));
		g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_CHIP));
		
		pPlayer->bNeedToAct = false;
		RefreshNeedToAct(pPlayer);
		pPlayer->penBet += penAmount;
		Progress();
	}

	if (decision == "All In")
	{
		if (pPlayer->penStack < 1)
		{
			SendF(accountId, "Your stack is empty!");
			return;
		}

		BroadcastF("%s moves all in for $%.2f.", pPlayer->name.c_str(), PENNY_F(pPlayer->penStack));
		g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_CHIP));
		
		pPlayer->bNeedToAct = false;
		RefreshNeedToAct(pPlayer);
		pPlayer->penBet += ExtractBet(pPlayer, pPlayer->penStack);
		Progress();
	}

	// Remove possible decisions, it will be refilled next update loop

	pPlayer->vDecisionChoices.clear();
}

// ------------------
// Progress
void Game::Progress()
{	
	// If no one has a stack left then no one needs to act

	if (NumPlayersWithStack() < 1)
		for (uint8 i = 0; i < NUM_SEATS; ++i)
			if (m_seats[i].pPlayer)
				m_seats[i].pPlayer->bNeedToAct = false;

	// Move to next stage if no one needs to act

	if (NumNeedToAct() <= 0)
	{		
		ScoopUpBets();
		RefreshNeedToAct();

		if (m_uiCurrentStage < FLOP)
			ProgressToFlop();
		else if (m_uiCurrentStage < TURN)
			ProgressToTurn();
		else if (m_uiCurrentStage < RIVER)
			ProgressToRiver();
		else
		{
			ProgressToShowdown();
			return;
		}
		
		// First to act in a rotation post-flop is always first seat after dealer

		m_uiStageTimer = 2000;
		m_uiSeatToAct = m_uiDealerSeat;

		// If only 1 guy can act after progressing, then no one needs to act

		if (NumPlayersWithStack(true, true) <= 1)
		{
			for (uint8 i = 0; i < NUM_SEATS; ++i)
				if (m_seats[i].pPlayer)
					m_seats[i].pPlayer->bNeedToAct = false;

			m_bSkipToShowdown = true;
			return;
		}
	}

	NextSeatToAct();
}

// -------------------
// ProgressToFlop
void Game::ProgressToFlop()
{
	// burn 1, draw 3
	m_cardDeck.DrawCard();
	m_board.push_back(m_cardDeck.DrawCard());
	m_board.push_back(m_cardDeck.DrawCard());
	m_board.push_back(m_cardDeck.DrawCard());

	m_uiCurrentStage = FLOP;

	BroadcastF("*** FLOP *** [%s %s %s]", 
		m_cardDeck.getCardName(m_board[0]).c_str(),
		m_cardDeck.getCardName(m_board[1]).c_str(),
		m_cardDeck.getCardName(m_board[2]).c_str());

	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_FLOP));
}

// -------------------
// ProgressToTurn
void Game::ProgressToTurn()
{
	// burn 1, draw 1
	m_cardDeck.DrawCard();
	m_board.push_back(m_cardDeck.DrawCard());
	
	m_uiCurrentStage = TURN;

	BroadcastF("*** TURN *** [%s %s %s] [%s]", 
		m_cardDeck.getCardName(m_board[0]).c_str(),
		m_cardDeck.getCardName(m_board[1]).c_str(),
		m_cardDeck.getCardName(m_board[2]).c_str(),
		m_cardDeck.getCardName(m_board[3]).c_str());

	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_NEWCARD));
}

// --------------------
// ProgressToRiver
void Game::ProgressToRiver()
{
	// burn 1, draw 1
	m_cardDeck.DrawCard();
	m_board.push_back(m_cardDeck.DrawCard());
	
	m_uiCurrentStage = RIVER;

	BroadcastF("*** TURN *** [%s %s %s %s] [%s]", 
		m_cardDeck.getCardName(m_board[0]).c_str(),
		m_cardDeck.getCardName(m_board[1]).c_str(),
		m_cardDeck.getCardName(m_board[2]).c_str(),
		m_cardDeck.getCardName(m_board[3]).c_str(),
		m_cardDeck.getCardName(m_board[4]).c_str());

	g_gameFloor.BroadcastPacket(m_gameBuffer, PackOpcode(m_gameBuffer, OPCODE_HAND_NEWCARD));
}

// --------------------
// ProgressToShowdown
void Game::ProgressToShowdown()
{
	AwardWinners();
}

// --------------------------
// BroadcastPlayerHand
void Game::BroadcastPlayerHand(Player* pPlayer)
{	
	std::vector<Card> cards;

	cards.push_back(pPlayer->cards[0]);
	cards.push_back(pPlayer->cards[1]);

	g_gameFloor.BroadcastPacket(m_gameBuffer, Session::BuildPlayerCardInfo(m_gameBuffer, pPlayer->accountId, cards));
}

// --------------------------
// AffectBankroll
//		'penBankrollChange' is how much to affect the banroll by, positive or negative acceptable
void Game::AffectBankroll(Player* pPlayer, PENNY penBankrollChange)
{
	uint8 uiAvatar = 0;

	// Update in memory
	if (Session* pSession = g_gameFloor.GetSession(pPlayer->accountId))
	{
		uiAvatar = pSession->getAvatar();
		pSession->affectBankroll(penBankrollChange);
	}

	Session::SaveAccount(pPlayer->name, uiAvatar, penBankrollChange);
}