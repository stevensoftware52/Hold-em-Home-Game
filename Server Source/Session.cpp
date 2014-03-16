#include "stdafx.h"

// --------------
// Constructor
Session::Session(TcpConnection *tcp, unsigned int accountId, float fBankroll, uint8 uiAvatar, std::string username)
{
    m_tcp = tcp;
    m_accountId = accountId;
    m_lastMsg = clock();
	m_uiAvatar = uiAvatar;
	m_name = username;  
	m_penBankroll = PENNY(fBankroll * 100.0f);  
}

// --------------
// Destructor
Session::~Session()
{

}

// ----------------
// SendMessageTo
void Session::SendMessageTo(unsigned int size, char *data)
{
    m_tcp->PushOutgoing(size, data);
}

// -----------------
// RespondToMessage
void Session::RespondToMessage(char *data, unsigned int size)
{
    // Update m_lastMsg
    m_lastMsg = clock();

    // Extract message type
    uint8 msgType = *(uint8 *)(data); 
	size -= sizeof(msgType);
    char *furtherData = data + sizeof(msgType);

    // Message types expected to originate from a client
    switch (msgType)
    {        
		case OPCODE_PING: break;
		case OPCODE_CHAT_MSG: ReceiveChatMsg(furtherData, size); break;
		case OPCODE_BUTTON_PRESS: ReceiveButtonPress(furtherData, size); break;

        default:            
            printf("Session (ACC: %d) processing unrecognized message (%d), checking for error - code %d\n", m_accountId, msgType, WSAGetLastError());
    }
}

// ---------------------
// Update
void Session::Update()
{
	char buffer[MAX_BUFFER_SIZE];

	// Send game information
	SendMessageTo(g_gameFloor.getGame()->PackAllTableInformation(buffer), buffer);

	// Send player information
	SendMessageTo(BuildPlayerInfo(buffer), buffer);
}