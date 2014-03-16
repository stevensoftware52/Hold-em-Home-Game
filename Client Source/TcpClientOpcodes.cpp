#include "stdafx.h"

// ---------------------------
// ProcessPacket
void TcpClient::ProcessPacket(Packet* packet)
{	
    // Extract opcode
    uint8 opcode = *(uint8 *)(packet->data); 
	unsigned int msgSize = packet->dataSize - sizeof(opcode);
    char *furtherData = packet->data + sizeof(opcode);

	switch (opcode)
	{
		case OPCODE_NULL: break;
		case OPCODE_PING: break;
		case OPCODE_ALL_TABLE_INFO: RecieveAllTableInformation(furtherData, msgSize); break;
		case OPCODE_CHAT_MSG: ReceiveChatMsg(furtherData, msgSize); break;
		case OPCODE_PLAYER_INFO: ReceivePlayerInfo(furtherData, msgSize); break;
		case OPCODE_PLAYER_CARDS: ReceivePlayerCards(furtherData, msgSize); break;
		case OPCODE_PLAYER_TAUNTS: ReceivePlayerTaunt(furtherData, msgSize); break;
		case OPCODE_PLAYER_PLAYMUSIC: ReceivePlayMusic(furtherData, msgSize); break;

		case OPCODE_HAND_OVER: 
		case OPCODE_HAND_CHIP:
		case OPCODE_HAND_MUCK:
		case OPCODE_HAND_CHECK:
		case OPCODE_HAND_NEWPLAYER:
		case OPCODE_HAND_DEAL:
		case OPCODE_HAND_NEWCARD:	
		case OPCODE_HAND_FLOP:	
			ReceiveHandOpcode(furtherData, opcode); 
			break;

        default:            
            printf("Unknown server opcode %d\n", opcode);
			break;
	}
}