#ifndef _TCPMGR_H
#define _TCPMGR_H

#include "stdafx.h"

// The port we listen() on
#define TCPMGR_LISTEN_PORT 7777

// FILE CONTENTS: Password \n AccountID \n Bankroll \n Avatar
#define NUM_USERFILE_CONTENTS 4

class TcpConnection;

class TcpMgr
{
    public:
        TcpMgr();
        ~TcpMgr();

        void Update();
        void NewTcpConnection(SOCKET sock, unsigned int account_id, float fBankroll, uint8 uiAvatar, std::string username);
                
        static void StaticListenThread(LPVOID lpParam); // Listens for an incoming connection then launches a RecvThread
        static void StaticAuthThread(LPVOID lpParam); // Accepts some data and checks if the received credentials authenticate, if does, then acts accordingly

		static unsigned int AuthenticateCredentials(std::string username, std::string password, float& fBankroll, uint8& avatar);

    private:    
		static unsigned int PumpNewAccountId();

        HANDLE m_mutex; 
        WSADATA m_wsaData;
        std::vector<TcpConnection*> m_vTcpConnections;
};

#endif