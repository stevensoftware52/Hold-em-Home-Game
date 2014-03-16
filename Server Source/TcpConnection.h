#ifndef _TCPCONN_H
#define _TCPCONN_H

#include "stdafx.h"

// Max size of a buffer, we'll say 64kb, because why not
#define MAX_BUFFER_SIZE 65536

struct Packet
{
    char data[MAX_BUFFER_SIZE];
    DWORD dataSize;
};

class Session;

class TcpConnection
{
    public:
        TcpConnection(unsigned int accountId, SOCKET sock, float fBankroll, uint8 uiAvatar, std::string username);
        ~TcpConnection();

        void ProcessPendingMsgs();
        void PushIncoming(unsigned int size, char *buffer);
        void PushOutgoing(unsigned int size, char *buffer);

        void threadEnded() { --m_activeThreads; } // ONLY called when a thread ends

        bool safeToDelete() const { return m_activeThreads == 0; } 

        unsigned int getActiveThreads() const { return m_activeThreads; }
        unsigned int getAccountId() const { return m_accountId; }

        static void SendThread(LPVOID lpParam);
        static void RecvThread(LPVOID lpParam);
        
        bool PopCopyOfOutgoing(Packet *pad);

        SOCKET m_socket;

    private:
        HANDLE m_mutex; 
        DWORD m_activeThreads;

        unsigned int m_accountId;

        Session* m_session;

        std::vector<Packet> m_outgoing;
        std::vector<Packet> m_incoming;
};

#endif