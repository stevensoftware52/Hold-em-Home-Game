#include "stdafx.h"

// ----------------------
// BuildChatMsg
unsigned int Session::BuildChatMsg(char* buffer, std::string msg, uint8 type)
{
	unsigned int bufferSize = 0;

	// Opcode (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_CHAT_MSG);

	// Message type (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, type);
		
	// Message string (uint8, string
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, msg.size());
	bufferSize += Util::bufferAddString(ONTO_BUFFER, msg.c_str());

	return bufferSize;
}

// ----------------------------
// BuildPotWinner
unsigned int Session::BuildPotWinner(char* buffer, unsigned int accountId, PENNY amount)
{
	unsigned int bufferSize = 0;

	// Opcode (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_POT_COLLECTOR);

	// AccountID (unsigned short)
	bufferSize += Util::bufferAddUShort(ONTO_BUFFER, accountId);

	// Amount (float)
	bufferSize += Util::bufferAddFloat(ONTO_BUFFER, PENNY_F(amount));

	return bufferSize;
}

// ---------------------
// BuildSendTaunt
unsigned int Session::BuildSendTaunt(char* buffer)
{
	unsigned int bufferSize = 0;

	// Opcode (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_PLAYER_TAUNTS);

	// Guid (ushort)
	bufferSize += Util::bufferAddUShort(ONTO_BUFFER, m_accountId);

	return bufferSize;
}

// -------------------------
// BuildPlayerInfo
unsigned int Session::BuildPlayerInfo(char* buffer)
{
	unsigned int bufferSize = 0;

	// Opcode (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_PLAYER_INFO);

	// AccountID (unsigned short)
	bufferSize += Util::bufferAddUShort(ONTO_BUFFER, m_accountId);

	// Username string (uint8, string)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, m_name.size());
	bufferSize += Util::bufferAddString(ONTO_BUFFER, m_name.c_str());

	// Bankroll (float)
	bufferSize += Util::bufferAddFloat(ONTO_BUFFER, PENNY_F(m_penBankroll));

	std::vector<std::string> buttonOptions = g_gameFloor.getGame()->GetAvailableDecisions(m_accountId);

	// Number of button options (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, buttonOptions.size());

	// The button options
	for (unsigned int i = 0; i < buttonOptions.size(); ++i)
	{
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, buttonOptions[i].size());
		bufferSize += Util::bufferAddString(ONTO_BUFFER, buttonOptions[i].c_str());
	}

	return bufferSize;
}

// ----------------------
// BuildPlayerCardInfo
unsigned int Session::BuildPlayerCardInfo(char* buffer, unsigned int accountId, std::vector<Card> cards)
{
	unsigned int bufferSize = 0;

	// Opcode (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_PLAYER_CARDS);

	// AccountID (unsigned short)
	bufferSize += Util::bufferAddUShort(ONTO_BUFFER, accountId);
	
	// Number of cards (uint8)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, cards.size());

	for (uint8 i = 0; i < cards.size(); ++i)
	{
		assert(cards[i].uiSuit && cards[i].uiValue);

		// Suit (uint8)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, cards[i].uiSuit);

		// Value (uint8)
		bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, cards[i].uiValue);
	}	

	return bufferSize;
}