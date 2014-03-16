#ifndef _GAMEFLOOR_H
#define _GAMEFLOOR_H

#include "stdafx.h"

// Miliseconds until a session times out
#define SESSION_TIMEOUT 10000

class Game;
class Session;

class GameFloor
{
    public:
        GameFloor();
        ~GameFloor();

        void Update(unsigned int uiDiff);
        void RemovePlayer(unsigned int accountId);

        Session* GetSession(unsigned int accountId) const;
        Session* GetSessionByName(const char *name) const;
                
        void AddSession(Session *session);
        void RemoveSession(unsigned int accountId);
		void BroadcastPacket(char* buffer, unsigned int size);

		Game* getGame() const { return m_game; }

    private:
        void UpdateSessions(unsigned int uiDiff);
        
		Game* m_game;

        HANDLE m_mutex;     
        std::vector<Session*> m_sessions;
};

#endif