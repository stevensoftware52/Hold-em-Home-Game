#include "stdafx.h"

#define UNPACK_CHECK { if (sizeUnpacked > size) { printf("Unexpected end of packet\n"); return; } }
#define UNPACK_UINT32 *(unsigned int *)(data + sizeUnpacked); { sizeUnpacked += sizeof(unsigned int); UNPACK_CHECK }
#define UNPACK_UINT16 *(unsigned short *)(data + sizeUnpacked); { sizeUnpacked += sizeof(unsigned short); UNPACK_CHECK }
#define UNPACK_UINT8 *(uint8 *)(data + sizeUnpacked); { sizeUnpacked += sizeof(uint8); UNPACK_CHECK }
#define UNPACK_CHAR *(char *)(data + sizeUnpacked); { sizeUnpacked += sizeof(char); UNPACK_CHECK }
#define UNPACK_BOOL *(bool *)(data + sizeUnpacked); { sizeUnpacked += sizeof(bool); UNPACK_CHECK }
#define UNPACK_FLOAT *(float *)(data + sizeUnpacked); { sizeUnpacked += sizeof(float); UNPACK_CHECK }

// -----------------------
// ReceiveChatMsg
void Session::ReceiveChatMsg(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	// Message string (uint8, string)
		
	uint8 msgSize = UNPACK_UINT8;
		
	std::string msg;

	for (uint8 i = 0; i < msgSize; ++i)
	{
		char letter = UNPACK_CHAR;
		msg.push_back(letter);
	}

	// Broadcast it right back out there
	//

	std::string newMsg;

	newMsg.append(getName());
	newMsg.append(": ");
	newMsg.append(msg);

	char buffer[1024];

	g_gameFloor.BroadcastPacket(buffer, BuildChatMsg(buffer, newMsg, CHATTYPE_PLAYER_MSG));

	// Chat commands
	//
	
	if (Player* pPlayer = g_gameFloor.getGame()->getPlayer(m_accountId))
	{
		if (msg.find("/taunt") == 0)
			g_gameFloor.BroadcastPacket(buffer, BuildSendTaunt(buffer));
	
		if (msg.find("/music") == 0)
			g_gameFloor.BroadcastPacket(buffer, g_gameFloor.getGame()->PackOpcode(buffer, OPCODE_PLAYER_PLAYMUSIC));

		if (msg.find("/avatar") == 0)
		{
			for (uint8 i = 0; i < strlen("/avatar"); ++i)
				msg.erase(msg.begin());

			m_uiAvatar = atoi(msg.c_str());		
			Session::SaveAccount(pPlayer->name, m_uiAvatar, 0);
		}
	}
}

// ----------------------
// ReceiveButtonPress
void Session::ReceiveButtonPress(char* data, unsigned int size)
{
	unsigned int sizeUnpacked = 0;
	
	// Button string (uint8, string)
		
	uint8 stringSize = UNPACK_UINT8;
		
	std::string button;

	for (uint8 i = 0; i < stringSize; ++i)
	{
		char letter = UNPACK_CHAR;
		button.push_back(letter);
	}

	// Amount (float)

	float fAmount = UNPACK_FLOAT; // NYI

	g_gameFloor.getGame()->ProcessPlayerDecision(m_accountId, button, fAmount);
}