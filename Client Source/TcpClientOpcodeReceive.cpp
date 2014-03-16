#include "stdafx.h"

#define UNPACK_CHECK { if (sizeUnpacked > size) { printf("Unexpected end of packet\n"); } }
#define UNPACK_UINT32 *(unsigned int *)(data + sizeUnpacked); { sizeUnpacked += sizeof(unsigned int); UNPACK_CHECK }
#define UNPACK_UINT16 *(unsigned short *)(data + sizeUnpacked); { sizeUnpacked += sizeof(unsigned short); UNPACK_CHECK }
#define UNPACK_UINT8 *(uint8 *)(data + sizeUnpacked); { sizeUnpacked += sizeof(uint8); UNPACK_CHECK }
#define UNPACK_CHAR *(char *)(data + sizeUnpacked); { sizeUnpacked += sizeof(char); UNPACK_CHECK }
#define UNPACK_BOOL *(bool *)(data + sizeUnpacked); { sizeUnpacked += sizeof(bool); UNPACK_CHECK }
#define UNPACK_FLOAT *(float *)(data + sizeUnpacked); { sizeUnpacked += sizeof(float); UNPACK_CHECK }

// ---------------------------------
// RecieveAllTableInformation
void TcpClient::RecieveAllTableInformation(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	g_tableRender.ResetBoard();
	g_tableRender.RemoveAllPlayers();
	g_tableRender.ResetBoard();

	// Number of players (uint8)
	uint8 numPlayers = UNPACK_UINT8;

	// For every seat where there's a player...
	for (uint8 i = 0; i < numPlayers; ++i)
	{		
		// Player name (uint8, string)
		uint8 nameSize = UNPACK_UINT8;
		
		std::string playerName;

		for (uint8 n = 0; n < nameSize; ++n)
		{
			char letter = UNPACK_CHAR;
			playerName.push_back(letter);
		}
		
		// Player GUID (ushort)
		unsigned short playerGUID = UNPACK_UINT16;
		
		// Time left for making a decision (ushort)
		unsigned short playerDecisionTime = UNPACK_UINT16;
		
		// Time bank (ushort)
		unsigned short playerTimeBank = UNPACK_UINT16;

		// Whether or not has posted BB (bool)
		bool playerPosted = UNPACK_BOOL;

		// Stack (float)
		float playerStack = UNPACK_FLOAT;

		// Bet (float)
		float playerBet = UNPACK_FLOAT;

		// Whether or not this player has a holding (bool)
		bool playerHasHolding = UNPACK_BOOL;

		// The seat he's in (uint8)
		uint8 playerSeat = UNPACK_UINT8;

		// His avatar (uint8)
		uint8 playerAvatar = UNPACK_UINT8;

		// Whether or not he is in the hand (bool)
		bool playerInHand = UNPACK_BOOL;
		
		// Add him
		
		Player* pPlayer = new Player(playerSeat, playerName, playerGUID, playerStack, playerAvatar);
		
		pPlayer->setBet(playerBet);
		pPlayer->setHolding(playerHasHolding);
		pPlayer->setIsInHand(playerInHand);
		pPlayer->m_decisionTime = playerDecisionTime / 1000;
		pPlayer->m_timeBank = playerTimeBank / 1000;

		g_tableRender.AddPlayer(playerSeat, pPlayer);
	}

	// Seat who's turn it is (uint8)
	g_gameInfo.m_uiSeatToAct = UNPACK_UINT8;

	// Seat that has button	(uint8)
	uint8 seatWithButton = UNPACK_UINT8;

	g_tableRender.setDealerSeat(seatWithButton);

	// Number of cards on board (uint8)
	uint8 numCardsOnBoard = UNPACK_UINT8;

	// Each card on board
	for (uint8 i = 0; i < numCardsOnBoard; ++i)
	{
		// Suit (uint8)
		uint8 cardSuit = UNPACK_UINT8;

		// Value (uint8)
		uint8 cardValue = UNPACK_UINT8;

		g_tableRender.AddBoardCard(g_cardLibrary.getCard(cardSuit, cardValue));
	}	

	// The pot (float)
	float fPot = UNPACK_FLOAT;
	
	g_tableRender.setPot(fPot);
}

// ------------------
// ReceiveChatMsg
void TcpClient::ReceiveChatMsg(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	// Chat type (uint8)
	uint8 chatType = UNPACK_UINT8;
	
	// Chat string (uint8, string)
		
	uint8 stringSize = UNPACK_UINT8;
		
	std::string msg;

	for (uint8 i = 0; i < stringSize; ++i)
	{
		char letter = UNPACK_CHAR;
		msg.push_back(letter);
	}

	g_chatBox.pushMsg(chatType, msg);
}

// --------------------------
// ReceiveBankrollInfo
void TcpClient::ReceivePlayerInfo(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	// AccountID (unsigned short)
	unsigned short myGUID = UNPACK_UINT16;
	
	g_gameInfo.m_uiGUID = myGUID;

	// Username string (uint8, string)
		
	uint8 stringSize = UNPACK_UINT8;
		
	std::string myUsername;

	for (uint8 i = 0; i < stringSize; ++i)
	{
		char letter = UNPACK_CHAR;
		myUsername.push_back(letter);
	}

	g_gameInfo.m_username = myUsername;

	// Bankroll (float)
	float myBankroll = UNPACK_FLOAT;

	g_gameInfo.m_fBankroll = myBankroll;

	// Number of button options (uint8)
	uint8 numButtons = UNPACK_UINT8;

	g_gameInfo.m_buttonOptions.clear();

	for (unsigned int i = 0; i < numButtons; ++i)
	{
		uint8 stringSize = UNPACK_UINT8;
		
		std::string button;

		for (uint8 i = 0; i < stringSize; ++i)
		{
			char letter = UNPACK_CHAR;
			button.push_back(letter);
		}

		g_gameInfo.m_buttonOptions.push_back(button);
	}

}

// ---------------------------
// ReceivePlayerCards
void TcpClient::ReceivePlayerCards(char* data, unsigned int size)
{
	PlayerCards playerCards;

	unsigned int sizeUnpacked = 0;
	
	// AccountID (unsigned short)
	playerCards.guid = UNPACK_UINT16;

	auto itr = g_gameInfo.m_playerCards.begin();

	while (itr != g_gameInfo.m_playerCards.end())
	{
		if (itr->guid == playerCards.guid)
			itr = g_gameInfo.m_playerCards.erase(itr);
		else
			++itr;
	}

	// Number of cards (uint8)
	uint8 numCards = UNPACK_UINT8;

	if (numCards < 1)
		return;

	// For each card...
	for (uint8 i = 0; i < numCards; ++i)
	{
		// Suit (uint8)
		uint8 suit = UNPACK_UINT8;
		
		// Value (uint8)
		uint8 value = UNPACK_UINT8;

		assert(suit && value);

		if (ClientCard* pCard = g_cardLibrary.getCard(suit, value))
			playerCards.cards.push_back(pCard);
	}

	g_gameInfo.m_playerCards.push_back(playerCards);
}

// ------------------------
// ReceiveHandIsOver
void TcpClient::ReceiveHandOpcode(char* buffer, uint8 opcode)
{
	std::string playsound;

	Vector2 dealerPosition = D_BUTTON_RENDER_V(g_tableRender.getDealerSeat());	

	switch (opcode)
	{
		case OPCODE_HAND_OVER: 
			g_gameInfo.m_playerCards.clear(); 
			g_gameInfo.m_cardAnimations.vBoard.clear();
			g_gameInfo.m_cardAnimations.vCardAnimations.clear(); 
			break;

		case OPCODE_HAND_CHIP:
			playsound = "chip.wav";
			break;

		case OPCODE_HAND_MUCK:
			playsound = "muck.wav";
			break;

		case OPCODE_HAND_CHECK:
			playsound = "check.wav";
			break;

		case OPCODE_HAND_NEWPLAYER:
			playsound = "player.wav";
			break;

		case OPCODE_HAND_DEAL:
		{
			playsound = "card.wav";		
			std::vector<Player*> players = g_tableRender.getPlayers();

			for (uint8 r = 0; r < 2; ++r)
			{
				for (uint8 i = 0; i < players.size(); ++i)
				{
					if (!players[i]->isInHand())
						continue;

					g_gameInfo.m_cardAnimations.vCardAnimations.push_back(CardAnimation(dealerPosition, (float(r) * -1.0f), players[i]->GetAnchor()));
				}
			}

			g_tableRender.setMsg("0.02");

			break;
		}
		case OPCODE_HAND_NEWCARD:	
			playsound = "card.wav";
			g_gameInfo.m_cardAnimations.vBoard = g_tableRender.getBoard();
			g_gameInfo.m_cardAnimations.vCardAnimations.clear();
			g_gameInfo.m_cardAnimations.vCardAnimations.push_back(CardAnimation(dealerPosition, 0.00f, Vector2(TABLE_CENTER_X, TABLE_CENTER_Y)));
			g_tableRender.setMsg("0.02");
			break;

		case OPCODE_HAND_FLOP:
			playsound = "card.wav"; // deal.wav ?

			g_gameInfo.m_cardAnimations.vCardAnimations.clear();
			g_gameInfo.m_cardAnimations.vCardAnimations.push_back(CardAnimation(dealerPosition, 0.0f, Vector2(TABLE_CENTER_X, TABLE_CENTER_Y)));
			g_gameInfo.m_cardAnimations.vCardAnimations.push_back(CardAnimation(dealerPosition, 0.1f, Vector2(TABLE_CENTER_X, TABLE_CENTER_Y)));
			g_gameInfo.m_cardAnimations.vCardAnimations.push_back(CardAnimation(dealerPosition, 0.2f, Vector2(TABLE_CENTER_X, TABLE_CENTER_Y)));
			break;

		case OPCODE_HAND_TIMEBANK:
			playsound = "time_bank.wav";
			break;
	}

	if (playsound.size())
		g_hgeClient.ToggleSound(playsound, true);
}

// -----------------------
// ReceivePlayerTaunt
void TcpClient::ReceivePlayerTaunt(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	// AccountID (unsigned short)
	unsigned short playerGUID = UNPACK_UINT16;

	PlayerTaunts taunt;

	taunt.durationLeft = 3.0f;
	taunt.guid = playerGUID;

	g_gameInfo.m_playerTaunts.push_back(taunt);

	if (Player* pPlayer = g_tableRender.GetPlayer(playerGUID))
	{
		char filename[256];
		sprintf_s(filename, "%d.wav", pPlayer->getAvatar());
		g_hgeClient.ToggleSound(filename, true, 25);
	}
}

// ---------------------
// ReceivePlayMusic
void TcpClient::ReceivePlayMusic(char* data, unsigned int size)
{
	g_tableRender.ToggleMusic(true);
}