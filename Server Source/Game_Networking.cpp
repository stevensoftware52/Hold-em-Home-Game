#include "stdafx.h"

// ----------------------
// PackHandIsOverOpcode
unsigned int Game::PackOpcode(char* buffer, uint8 opcode)
{
	unsigned int bufferSize = 0;

	// OpCode OPCODE_HAND_OVER (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, opcode);

	return bufferSize;
}

// --------------------------
// PackAllTableInformation
unsigned int Game::PackAllTableInformation(char* buffer)
{
	unsigned int bufferSize = 0;

	// OpCode OPCODE_ALL_TABLE_INFO (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_ALL_TABLE_INFO);

	// Number of players (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, NumPlayers());

	// For every seat where there's a player...
	for (uint8 i = 0; i < NUM_SEATS; ++i)
	{
		if (!m_seats[i].pPlayer)
			continue;
		
		// Player name (uint8, string)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_seats[i].pPlayer->name.size());
		bufferSize += Util::bufferAddString(ONTO_BUFFER, m_seats[i].pPlayer->name.c_str());
		
		// Player GUID (ushort)
		bufferSize += Util::bufferAddUShort(ONTO_BUFFER, m_seats[i].pPlayer->accountId);
		
		// Time left for making a decision (ushort)
		bufferSize += Util::bufferAddUShort(ONTO_BUFFER, (unsigned short)m_seats[i].pPlayer->dDecisionTime);
		
		// Time bank (ushort)
		bufferSize += Util::bufferAddUShort(ONTO_BUFFER, (unsigned short)m_seats[i].pPlayer->dTimeBank);

		// Whether or not has posted BB (bool)
		bufferSize += Util::bufferAddBool(ONTO_BUFFER, m_seats[i].pPlayer->bPostedBB);

		// Stack (float)
		bufferSize += Util::bufferAddFloat(ONTO_BUFFER, (float)m_seats[i].pPlayer->penStack / 100.0f);

		// Bet (float)
		bufferSize += Util::bufferAddFloat(ONTO_BUFFER, (float)m_seats[i].pPlayer->penBet / 100.0f);

		// Whether or not this player has a holding (bool)
		bufferSize += Util::bufferAddBool(ONTO_BUFFER, playerHasHolding(m_seats[i].pPlayer));	
		
		// The seat he's in (uint8)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, i);
		
		// His avatar (uint8)

		uint8 uiAvatar = 0;

		if (Session* pSession = g_gameFloor.GetSession(m_seats[i].pPlayer->accountId))
			uiAvatar = pSession->getAvatar();

		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, uiAvatar);

		// Whether or not he is in the hand (bool)
		bufferSize += Util::bufferAddBool(ONTO_BUFFER, m_seats[i].pPlayer->penStackBeforeHand > 0);
	}

	// Seat who's turn it is (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, !handInProgress() ? NUM_SEATS : m_uiSeatToAct);

	// Seat that has button (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_uiDealerSeat);	

	// Number of cards on board (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_board.size());

	// Each card on board
	for (uint8 i = 0; i < m_board.size(); ++i)
	{
		if (m_board[i].uiSuit <= 0)
			continue;

		// Suit (uint8)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_board[i].uiSuit);

		// Value (uint8)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_board[i].uiValue);
	}	

	// The pot (float)
	bufferSize += Util::bufferAddFloat(ONTO_BUFFER, float(m_penPot) / 100.0f);	

	return bufferSize;
}