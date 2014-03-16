#ifndef SESSION_H
#define SESSION_H

#include "stdafx.h"

class TcpConnection;

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
};

enum ChatTypes
{
	CHATTYPE_NULL,
	CHATTYPE_SERVER_MSG,
	CHATTYPE_PLAYER_MSG,
	CHATTYPE_DIRECT_SERVER_MSG,
};

#define ONTO_BUFFER (buffer + bufferSize)

class Session
{
    public:
        Session(TcpConnection *tcp, unsigned int accoundId, float fBankroll, uint8 uiAvatar, std::string username);
        ~Session();

		void Update();
        void RespondToMessage(char *data, unsigned int size);
        void SendMessageTo(unsigned int size, char *data);
		
		void affectBankroll(PENNY amount) { m_penBankroll += amount; }

		// Game stuff

		uint8 getAvatar() const { return m_uiAvatar; }
		PENNY getBankrollP() const { return m_penBankroll; }

		// Tcp stuff
		
        DWORD lastMsgTimeStamp() const { return m_lastMsg; }
        unsigned int getAccountId() const { return m_accountId; }		
        TcpConnection *getTcp() { return m_tcp; }
		const char* getName() const { return m_name.c_str(); }
		
		// Send OPCODE's
		//
		
		unsigned int BuildPlayerInfo(char* buffer);
		unsigned int BuildSendTaunt(char* buffer);
		static unsigned int BuildPlayerCardInfo(char* buffer, unsigned int accountId, std::vector<Card> cards);
		static unsigned int BuildChatMsg(char* buffer, std::string msg, uint8 type);		
		
		// Recv OPCODE's
		//

		void ReceiveChatMsg(char* data, unsigned int size);
		void ReceiveButtonPress(char* data, unsigned int size);

    private:
		
		uint8 m_uiAvatar;
		PENNY m_penBankroll;

        DWORD m_lastMsg;
		std::string m_name;
        unsigned int m_accountId;
        TcpConnection* m_tcp;
        
};

#endif