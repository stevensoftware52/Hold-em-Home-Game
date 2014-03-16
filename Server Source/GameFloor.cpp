#include "stdafx.h"

// ------------
// Constructor
GameFloor::GameFloor()
{
    printf("Creating game floor.\n");

	m_game = new Game();
    m_mutex = CreateMutex(NULL, FALSE, NULL); 
}

// ------------
// Destructor
GameFloor::~GameFloor()
{
    CloseHandle(m_mutex);
	delete m_game;
}

// --------
// Update
void GameFloor::Update(unsigned int uiDiff)
{    
    WaitForSingleObject(m_mutex, INFINITE);

    {
        // Update m_sessions
        UpdateSessions(uiDiff);

		// Update m_game
		m_game->Update(uiDiff);
    }
    
    ReleaseMutex(m_mutex);
}

// --------------
// UpdateSessions
void GameFloor::UpdateSessions(unsigned int uiDiff)
{
    auto it = m_sessions.begin();

    while (it != m_sessions.end())
    {
        if (Session *pSession = (*it))
        {
            if (clock() - pSession->lastMsgTimeStamp() > SESSION_TIMEOUT)
            {				
                // Closing the socket will cause its thread to fall out of scope, 
                // which will cause its parent TcpConnection to be deleted by TcpMgr,
                // which will then delete pSession session from memory.
                closesocket(pSession->getTcp()->m_socket); 
                pSession->getTcp()->m_socket = INVALID_SOCKET;

                it = m_sessions.erase(it);
            }
            else
            {
				pSession->Update();
                ++it;
            }
        }
    }
}

// --------------
// AddSession
void GameFloor::AddSession(Session *session)
{ 
	// Remove pre-existing sessions of same account
	// Note that ::RemoveSession grabs m_mutex so do this outside of the content below

	RemoveSession(session->getAccountId()); 
	
	// Ready

    WaitForSingleObject(m_mutex, INFINITE);

    {  
		printf("Player '%s' joins the game floor.\n", session->getName());
        m_sessions.push_back(session); 
    }
            
    ReleaseMutex(m_mutex);
}

// ----------------------------
// RemoveSession
void GameFloor::RemoveSession(unsigned int accountId)
{		
    WaitForSingleObject(m_mutex, INFINITE);

	{
		for (auto itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
		{
			if ((*itr)->getAccountId() == accountId)
            {
				Session* pSession = (*itr);

				printf("%s leaves game floor.\n", pSession->getName());

                closesocket(pSession->getTcp()->m_socket);
                pSession->getTcp()->m_socket = INVALID_SOCKET;
                m_sessions.erase(itr);     
                break;
            }
        }
	}

    ReleaseMutex(m_mutex);
}

// --------------------
// GetSession
Session* GameFloor::GetSession(unsigned int accountId) const
{
    for (unsigned int i = 0; i < m_sessions.size(); i++)
        if (m_sessions[i]->getAccountId() == accountId)
            return m_sessions[i];

    return 0;
}

// ----------------
// GetSessionByName
Session* GameFloor::GetSessionByName(const char* name) const
{
    for (unsigned int i = 0; i < m_sessions.size(); i++)
        if (m_sessions[i]->getName() == name)
            return m_sessions[i];

    return 0;
}

// --------------------
// BroadcastPacket
void GameFloor::BroadcastPacket(char* buffer, unsigned int size)
{
    for (unsigned int i = 0; i < m_sessions.size(); i++)
		m_sessions[i]->SendMessageTo(size, buffer);
}