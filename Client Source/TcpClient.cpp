#include "stdafx.h"

// -----------------
// TcpClient
//		It's not ideal to spam grab a mutex, but it's
//		not going to make any difference in this situation
void TcpClient::Update(float fDelta)
{	
    WaitForSingleObject(m_mutex, INFINITE);
	
	{
		if (!g_tcpClient.m_bConnected)
		{
			m_incomingPackets.clear();
			m_outgoingPackets.clear();
			return;
		}

		// Ping timer
		//

		m_fPingTimer += fDelta;

		if (m_fPingTimer >= PING_INTERVAL)
		{
			SendPing();
			m_fPingTimer = 0.0f;
		}

		// Process packets
		//

		while (m_incomingPackets.size())
		{
			ProcessPacket(&m_incomingPackets[0]);
			m_incomingPackets.erase(m_incomingPackets.begin());
		}
	}

    ReleaseMutex(m_mutex);
}

// ----------------
// PushOutgoingMsg
void TcpClient::PushOutgoingMsg(char *buffer, unsigned int size)
{
    WaitForSingleObject(m_mutex, INFINITE);

    {
        Packet packet;
        memcpy(packet.data, buffer, size);
        packet.dataSize = size;

        m_outgoingPackets.push_back(packet);        
    }

    ReleaseMutex(m_mutex);
}

// -------------------
// PopCopyOfOutgoing
//      Copies top of m_outgoing into *pad then deletes top of m_outgoing
bool TcpClient::PopCopyOfOutgoing(Packet *pad)
{
    WaitForSingleObject(m_mutex, INFINITE);

    if (m_outgoingPackets.size())
    {
        memcpy(pad->data, m_outgoingPackets[0].data, m_outgoingPackets[0].dataSize);
        pad->dataSize = m_outgoingPackets[0].dataSize;
        m_outgoingPackets.erase(m_outgoingPackets.begin());
    }
    else
        pad->dataSize = 0;

    return ReleaseMutex(m_mutex) > 0;
}

// ----------------
// PushIncomingMsg
void TcpClient::PushIncomingMsg(char *data, unsigned int datasize)
{
    WaitForSingleObject(m_mutex, INFINITE);

    {
        if (datasize >= 1 && datasize <= MAX_BUFFER_SIZE)
        {
            Packet packet;
            memcpy(packet.data, data, datasize);
            packet.dataSize = datasize;

            m_incomingPackets.push_back(packet);
        }
        else
        {
            printf("ERROR: bad msg size %d\n", datasize);
        }
    }
    
    ReleaseMutex(m_mutex);
}

// ----------------------
// AttemptConnection
void TcpClient::AttemptConnection(std::string username, std::string password)
{
	// Extract address we're connecting to from ServerAddress.txt
	//

	std::string serverAddrStr;

	ifstream myfile (SERVER_ADDR_TXT_FNAME);

	if (myfile.is_open())
	{
		getline(myfile, serverAddrStr);
		myfile.close();
	}
	else
		printf("TcpClient: Missing file %s\n", SERVER_ADDR_TXT_FNAME);

	// Create socket
	//

	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sSocket == INVALID_SOCKET)
    {
        printf("TcpClient: socket() failed: %d\n", WSAGetLastError());
        return;
    }

	// Prepare address in form of sockaddr_in
	//

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(serverAddrStr.c_str());

	// Try to resolve hostname if need to
	//

    if (serverAddr.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *host = gethostbyname(serverAddrStr.c_str());

        if (!host) 
        {
            printf("TcpClient: Failed to resolve hostname\n");
            return;
        }

        CopyMemory(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);
    }

	// Connect
	//
	
    if (connect(sSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("TcpClient: connect() failed: %d\n", WSAGetLastError());
        return;
    }

	// Connected, begin authentication
	//

	{
		char* buffer = new char[MAX_BUFFER_SIZE];
		unsigned int size = 0;
    
		// 1 byte for len of username
		size += Util::bufferAddChar(buffer + size, username.size()); 

		// x bytes for username
		size += Util::bufferAddString(buffer + size, username.c_str()); 
    
		// 1 byte for len of password
		size += Util::bufferAddChar(buffer + size, password.size()); 

		// x bytes for password
		size += Util::bufferAddString(buffer + size, password.c_str()); 

		if (Util::sendBytes(sSocket, buffer, (int)size) == SOCKET_ERROR)
			g_tcpClient.m_bConnected = false;

		// Connected to game server
		else
		{
			g_tcpClient.m_bConnected = true;

			g_chatBox.pushMsg(CHATTYPE_SERVER_MSG, "Welcome to the MF & Sons' HomeGame!");
			g_gameInfo.clearPlayerCards();

			_beginthread(RecvThread, 0, (LPVOID)sSocket);
			_beginthread(SendThread, 0, (LPVOID)sSocket);
		}
    
		delete [] buffer;
	}
}

// ---------------
// RecvThread
//      Static function
void TcpClient::RecvThread(LPVOID lpParam)
{
    SOCKET sSocket = (SOCKET)lpParam;

    while (true)
    {
        char buffer[4];

        // How long is the next message?
        if (Util::recvBytes(sSocket, buffer, 4) == 4)
        {
            DWORD msgSize = *(DWORD*)buffer;

            if (msgSize <= MAX_BUFFER_SIZE)
            {
                char* msg = new char[msgSize];

                // Recv the next message of msgSize length
                if (Util::recvBytes(sSocket, msg, msgSize) == msgSize)
                {
                    g_tcpClient.PushIncomingMsg(msg, msgSize);   
                    delete [] msg;
                }
                else
                {
                    delete [] msg;
                    break;
                }
            }
            else
                break;
        }
        else
            break;
    }
    
	g_tcpClient.m_bConnected = false;
}

// ---------------
// SendThread
//      Static function
void TcpClient::SendThread(LPVOID lpParam)
{
    SOCKET sSocket = (SOCKET)lpParam;

    char* outgoing = 0;
    unsigned int outgoingSize = 0;

    while (g_tcpClient.m_bConnected)
    {
        outgoing = new char[MAX_BUFFER_SIZE];
        outgoingSize = 0;

        Packet packet;
        g_tcpClient.PopCopyOfOutgoing(&packet);

        // Pack into outgoing until there's nothing left or we run out of space
        while (packet.dataSize && outgoingSize + packet.dataSize < MAX_BUFFER_SIZE)
        {
			// Len of msg
			char lenOfMsg[4];
			Util::bufferAddInt(lenOfMsg, packet.dataSize);
			memcpy(outgoing + outgoingSize, lenOfMsg, 4);

			// Increment size
			outgoingSize += 4;

            // Copy 
            memcpy(outgoing + outgoingSize, packet.data, packet.dataSize);

            // Increment size
            outgoingSize += packet.dataSize;
            
            // Grab next
            g_tcpClient.PopCopyOfOutgoing(&packet);
        }

        if (outgoingSize)
        {            
            // outgoing
            if (Util::sendBytes(sSocket, outgoing, outgoingSize) == SOCKET_ERROR)
                break;
        }
        
        delete [] outgoing;
        outgoing = 0;

        Sleep(1);
    }

    delete [] outgoing;
}