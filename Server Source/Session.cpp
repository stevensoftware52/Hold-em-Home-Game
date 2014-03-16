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

// ------------------
// SaveAccount
void Session::SaveAccount(std::string name, uint8 uiAvatar, PENNY penBankRollChange)
{
	char filename[256];
	sprintf_s(filename, "Users\\%s.txt", name.c_str());
		
	ifstream myfile(filename);

	std::vector<string> userInformation;

	// Update in disk
	//

	// Read file contents
	if (myfile.is_open())
	{
		std::string line;

		while (getline (myfile, line))
		{
			userInformation.push_back(line);

			if (userInformation.size() >= NUM_USERFILE_CONTENTS)
				break;
		}

		// Check data
		if (userInformation.size() != NUM_USERFILE_CONTENTS)
		{
			printf("Unexpected end of user contents\n");	
			myfile.close();
			return;
		}
		// Data is good, go for the gold
		else
		{
			// Modify bankroll
			float fBankroll = (float)atof(userInformation[2].c_str());
			fBankroll += PENNY_F(penBankRollChange);			
			userInformation[2] = std::to_string(fBankroll);
		}

		myfile.close();
	}

	// Overwrite file contents
	if (userInformation.size() == NUM_USERFILE_CONTENTS)
	{
		ofstream filestr;

		{
			filestr.open(filename, fstream::in | fstream::out | ios::trunc);

			if (filestr.is_open())
			{
				std::string avatar = uiAvatar ? std::to_string(uiAvatar) : userInformation[3];

				filestr 

					// Username
					<< userInformation[0] 
					<< '\n' 

					// Password
					<< userInformation[1] 
					<< '\n'

					// Money
					<< userInformation[2] 
					<< '\n' 

					// Avatar
					<< avatar;
				
				
				filestr.close();
			}
			else
			{
				printf("CRASH: User's file corrupted!!\n");
				assert(0);
			}
		}
	}
}