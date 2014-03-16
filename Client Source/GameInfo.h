#ifndef _GAMEINFO_H
#define _GAMEINFO_H

#include "stdafx.h"

struct PlayerCards
{
	unsigned int guid;
	std::vector<ClientCard*> cards;
};

struct PlayerTaunts
{
	unsigned int guid;
	float durationLeft;
};

struct CardAnimation
{
	CardAnimation() {};
	CardAnimation(Vector2 sp, float p, Vector2 dest) : startPos(sp), progression(p), endPos(dest) {}
	
	Vector2 startPos;
	Vector2 endPos;
	float progression;
};

struct CardAnimations
{
	std::vector<ClientCard*> vBoard;
	std::vector<CardAnimation> vCardAnimations;
};

class GameInfo
{
	public:
		GameInfo() 
		{
			m_fBankroll = 0;
			m_username = "Unknown";
			m_uiGUID = 0;
			m_uiSeatToAct = 0;
			m_fBigBlind = 0.02f; // TODO: Get this from server
		}
		
		void Update();
		void RenderText();
		void PumpDrawCardAnimation();

		void clearPlayerCards() { m_playerCards.clear(); }
		
		bool buttonIsOption(std::string title) const 
		{ 
			return find(m_buttonOptions.begin(), m_buttonOptions.end(), title) != m_buttonOptions.end(); 
		}

		bool isTaunting(unsigned int guid) const
		{
			for (unsigned int i = 0; i < m_playerTaunts.size(); ++i)
				if (m_playerTaunts[i].guid == guid)
					return true;
			return false;
		}

		std::vector<ClientCard*> getPlayerCards(unsigned int guid) const
		{
			for (unsigned int i = 0; i < m_playerCards.size(); ++i)
				if (m_playerCards[i].guid == guid)
					return m_playerCards[i].cards;
		}
		
		float m_fBigBlind;
		float m_fBankroll;
		
		std::string m_username;

		uint8 m_uiSeatToAct;

		unsigned short m_uiGUID;

		std::vector<PlayerCards> m_playerCards;
		std::vector<std::string> m_buttonOptions;
		std::vector<PlayerTaunts> m_playerTaunts;

		CardAnimations m_cardAnimations;
};

#endif