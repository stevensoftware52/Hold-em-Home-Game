#include "..\\stdafx.h"

#define ONTO_BUFFER (buffer + bufferSize)

// ---------------------------------
// SendPing ( OPCODE_PING )
void TcpClient::SendPing()
{
	if (!m_bConnected)
		return;

    char buffer[4];
    unsigned int size = 0;
    
    // Opcode
    size += Util::bufferAddUINT8(buffer, OPCODE_PING);

    // Ship it!
    PushOutgoingMsg(buffer, size);
}

// ----------------------
// SendChatMsg
void TcpClient::SendChatMsg(std::string msg)
{
	if (msg.size() >= 1024)
		return;

    char buffer[1028];
    unsigned int bufferSize = 0;
	    
    // Opcode (uint8)
    bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_CHAT_MSG);

	// Message string (uint8, string)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, msg.size());
	bufferSize += Util::bufferAddString(ONTO_BUFFER, msg.c_str());

    // Ship it!
    PushOutgoingMsg(buffer, bufferSize);
}

// ---------------------
// SendButtonPress
void TcpClient::SendButtonPress(std::string button)
{
	if (button.size() >= 124)
		return;

    char buffer[128];
    unsigned int bufferSize = 0;
	    
    // Opcode (uint8)
    bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, OPCODE_BUTTON_PRESS);

	// Button string (uint8, string)
	bufferSize += Util::bufferAddUINT8(ONTO_BUFFER, button.size());
	bufferSize += Util::bufferAddString(ONTO_BUFFER, button.c_str());
	    
    // Amount in text box (float)
    bufferSize += Util::bufferAddFloat(ONTO_BUFFER, g_tableRender.getTypedAmount());

    // Ship it!
    PushOutgoingMsg(buffer, bufferSize);
}