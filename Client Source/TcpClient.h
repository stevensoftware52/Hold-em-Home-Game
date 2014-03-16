#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include "stdafx.h"

// Port we try to connect to
#define SERVER_PORT 7777

// Name of file we extract address we're connecting to 
#define SERVER_ADDR_TXT_FNAME "Content\\ServerAddress.txt"

// Max size of a buffer, we'll say 64kb, because why not
#define MAX_BUFFER_SIZE 65536

// How often we ping, in seconds
#define PING_INTERVAL 1.0f

enum OpCodes
{
	OPCODE_NULL,
	OPCODE_PING,
	OPCODE_BUTTON_PRESS,
	OPCODE_ALL_TABLE_INFO,
	OPCODE_CHAT_MSG,
	OPCODE_PLAYER_INFO,
	OPCODE_PLAYER_CARDS,
	OPCODE_PLAYER_TAUNTS,
	OPCODE_PLAYER_PLAYMUSIC,

	OPCODE_HAND_OVER,
	OPCODE_HAND_CHIP,
	OPCODE_HAND_MUCK,
	OPCODE_HAND_CHECK,
	OPCODE_HAND_NEWPLAYER,
	OPCODE_HAND_DEAL,
	OPCODE_HAND_NEWCARD,
	OPCODE_HAND_FLOP,
	OPCODE_HAND_TIMEBANK
};

struct Packet
{
    Packet() : 
		dataSize(0) {}

    char data[MAX_BUFFER_SIZE];
    DWORD dataSize;
};

class TcpClient
{
	public:		
		TcpClient() :
			m_bConnected(false)
		{
			WSAStartup(MAKEWORD(2, 0), &m_wsa);
			m_mutex = CreateMutex(NULL, FALSE, NULL); 	
		}

		~TcpClient()
		{
			CloseHandle(m_mutex);
		}

		void Update(float fDelta);
        void PushOutgoingMsg(char *buffer, unsigned int size);
        void PushIncomingMsg(char *data, unsigned int size);
		void ProcessPacket(Packet* packet);

        bool PopCopyOfOutgoing(Packet* pad);

		// We're not particularly concerned with security, just send user/pw as raw data
		static void AttemptConnection(std::string username, std::string password);	

		// Functions that exist in their own thread
		static void RecvThread(LPVOID lpParam);
		static void SendThread(LPVOID lpParam);		

		// Threads access this variable. Volatile isn't threadproof but even a regular bool works fine for how we're using it
		volatile bool m_bConnected;

		// Send OPCODE's
		//
		
		void SendPing();
		void SendChatMsg(std::string msg);
		void SendButtonPress(std::string button);

		// Recv OPCODE's
		//

		void RecieveAllTableInformation(char* data, unsigned int msgSize);
		void ReceiveChatMsg(char* data, unsigned int msgSize);
		void ReceivePlayerInfo(char* data, unsigned int msgSize);
		void ReceivePlayerCards(char* data, unsigned int msgSize);
		void ReceiveHandOpcode(char* data, uint8 opcode);
		void ReceivePlayerTaunt(char* data, unsigned int msgSize);
		void ReceivePlayMusic(char* data, unsigned int msgSize);

	private:
        HANDLE m_mutex;
        WSADATA m_wsa;

		float m_fPingTimer;

        std::vector<Packet> m_incomingPackets;        
        std::vector<Packet> m_outgoingPackets;
};

#endif