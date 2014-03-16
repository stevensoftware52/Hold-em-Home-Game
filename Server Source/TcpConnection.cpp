#include "stdafx.h"

// -----------------
// Constructor
TcpConnection::TcpConnection(unsigned int accountId, SOCKET sock, float fBankroll, uint8 uiAvatar, std::string username)
{
    m_session = new Session(this, accountId, fBankroll, uiAvatar, username);
    m_accountId = accountId;
    m_socket = sock;

    m_mutex = CreateMutex(NULL, FALSE, NULL); 

    m_activeThreads = 2;
    
    _beginthread(SendThread, 0, (LPVOID)this);
    _beginthread(RecvThread, 0, (LPVOID)this);

    g_gameFloor.AddSession(m_session);
}

// -----------------
// Destructor
TcpConnection::~TcpConnection()
{
    delete m_session;
    CloseHandle(m_mutex);
}

// -------------------
// ProcessPendingMsgs
void TcpConnection::ProcessPendingMsgs()
{
    WaitForSingleObject(m_mutex, INFINITE);

    {
        while (m_incoming.size())
        {
            m_session->RespondToMessage((*m_incoming.begin()).data, (*m_incoming.begin()).dataSize);
            m_incoming.erase(m_incoming.begin());
        }
    }

    ReleaseMutex(m_mutex);
}

// -----------------
// PushIncoming
//      Copies buffer into a new Packet object
void TcpConnection::PushIncoming(unsigned int size, char *buffer)
{
    WaitForSingleObject(m_mutex, INFINITE);

    {
        Packet packet;
        memcpy(packet.data, buffer, size);
        packet.dataSize = size;

        m_incoming.push_back(packet);
    }

    ReleaseMutex(m_mutex);
}

// -----------------
// PushOutgoing
//      Copies buffer into a new Packet object
void TcpConnection::PushOutgoing(unsigned int size, char *buffer)
{
    WaitForSingleObject(m_mutex, INFINITE);

    {
        Packet packet;
        memcpy(packet.data, buffer, size);
        packet.dataSize = size;

        m_outgoing.push_back(packet);
    }

    ReleaseMutex(m_mutex);
}

// -----------------
// PopCopyOfOutgoing
//      Copies top of m_outgoing into *pad then deletes top of m_outgoing
bool TcpConnection::PopCopyOfOutgoing(Packet *pad)
{
    WaitForSingleObject(m_mutex, INFINITE);

    if (m_outgoing.size())
    {
        memcpy(pad->data, m_outgoing[0].data, m_outgoing[0].dataSize);
        pad->dataSize = m_outgoing[0].dataSize;
        m_outgoing.erase(m_outgoing.begin());
    }
    else
        pad->dataSize = 0;

    return ReleaseMutex(m_mutex) > 0;
}

// ---------------
// SendThread
//      Static function launched via _beginthread
void TcpConnection::SendThread(LPVOID lpParam)
{
    TcpConnection* pChild = (TcpConnection*)lpParam;

    char* outgoing = 0;
    unsigned int outgoingSize = 0;

    while (pChild->getActiveThreads() >= 2)
    {
        outgoing = new char[MAX_BUFFER_SIZE];
        outgoingSize = 0;

        Packet packet;
        pChild->PopCopyOfOutgoing(&packet);

        // Pack into outgoing until there's nothing left or we run out of space
        while (packet.dataSize && outgoingSize + packet.dataSize < MAX_BUFFER_SIZE)
        {
			// Len of msg
			char lenOfMsg[4];
			Util::bufferAddUInt(lenOfMsg, packet.dataSize);
			memcpy(outgoing + outgoingSize, lenOfMsg, 4);

			// Increment size
			outgoingSize += 4;

            // Msg 
            memcpy(outgoing + outgoingSize, packet.data, packet.dataSize);

            // Increment size
            outgoingSize += packet.dataSize;
            
            // Grab next
            pChild->PopCopyOfOutgoing(&packet);
        }

        if (outgoingSize)
        {        
            if (Util::sendBytes(pChild->m_socket, outgoing, outgoingSize) == SOCKET_ERROR)
                break;
        }
        
        delete [] outgoing;
        outgoing = 0;

        Sleep(1);
    }

    delete [] outgoing;
        
    pChild->threadEnded();
}

// ---------------
// RecvThread
//      Static function launched via _beginthread
void TcpConnection::RecvThread(LPVOID lpParam)
{
    TcpConnection* pChild = (TcpConnection*)lpParam;

    while (true)
    {
        char buffer[4];

        // How long is the next message?
        if (Util::recvBytes(pChild->m_socket, buffer, 4) == 4)
        {
            DWORD msgSize = *(DWORD*)buffer;

            if (msgSize <= MAX_BUFFER_SIZE)
            {
                char* msg = new char[msgSize];

                // Recv the next message of msgSize length
                if (Util::recvBytes(pChild->m_socket, msg, msgSize) == msgSize)
                {
                    pChild->PushIncoming(msgSize, msg);   
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

    pChild->threadEnded();
}