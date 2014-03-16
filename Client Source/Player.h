#ifndef _PLAYER_H
#define _PLAYER_H

#include "stdafx.h"

// Number of cards a player holds
#define MAX_HOLD_CARDS 2

// Formula for where the player label box is rendered (hot spot must be centered !!)
#define PLAYER_LABEL_X(x) (x)
#define PLAYER_LABEL_Y(y) (y + (g_tableRender.getPlayerFrameSize() / 2.0f))
#define PLAYER_LABEL_V(x, y) Vector2(PLAYER_LABEL_X(x), PLAYER_LABEL_Y(y))

// Relative render positions of "the button" (the dealer chip)
static float g_d_buttonPosRelative[MAX_PLAYERS][2] = 
{
	// { CENTER_X + (Width * VALUE) } , { CENTER_Y + (Height * VALUE) }
			
		{ 0.34f, -0.175f},  // Right
		{  0.30f, 0.20f },  // Bottom Right 
		{ -0.1f, 0.27f },   // Bottom Left	
		{ -0.34f, 0.175f},  // Left
		{ -0.30f, -0.20f }, // Top Left 
		{  0.1f, -0.27f },  // Top Right	
};

// Formula to take g_buttonPosRelative and get a render position. (a) is the seat number, which should not be >= MAX_PLAYERS
#define D_BUTTON_RENDER_X(a) (float)(int)(TABLE_CENTER_X + (g_tableRender.getTableWidth() * g_d_buttonPosRelative[a][0]))
#define D_BUTTON_RENDER_Y(a) (float)(int)(TABLE_CENTER_Y + (g_tableRender.getTableHeight() * g_d_buttonPosRelative[a][1]))
#define D_BUTTON_RENDER_V(a) Vector2(D_BUTTON_RENDER_X(a), D_BUTTON_RENDER_Y(a))

// Formula to deduce scale of dealer button icon
#define D_BUTTON_ICON_SCALE(a) (((256.0f / a->GetWidth()) / 2.5f) * g_tableRender.getTableScale())

// Relative render positions of a player's face down holding
static float g_holdingPosRelative[MAX_PLAYERS][2] = 
{
	// { CENTER_X + (Width * VALUE) } , { CENTER_Y + (Height * VALUE) }
			
		{ 0.34f, 0.00f},    // Right	
		{  0.22f, 0.30f },  // Bottom Right	
		{ -0.22f, 0.30f },  // Bottom Left
		{ -0.34f, 0.00f},   // Left	
		{ -0.22f, -0.30f }, // Top Left		
		{  0.22f, -0.30f }, // Top Right	
};

// Formula to take g_holdingPosRelative and get a render position. (a) is the seat number, which should not be >= MAX_PLAYERS
#define HOLDING_RENDER_X(a) (float)(int)(TABLE_CENTER_X + (g_tableRender.getTableWidth() * g_holdingPosRelative[a][0]))
#define HOLDING_RENDER_Y(a) (float)(int)(TABLE_CENTER_Y + (g_tableRender.getTableHeight() * g_holdingPosRelative[a][1]))
#define HOLDING_RENDER_V(a) Vector2(HOLDING_RENDER_X(a), HOLDING_RENDER_Y(a))

// Formula to deduce scale of dealer button icon
#define HOLDING_ICON_SCALE(a) ((64.0f / a->GetWidth()) * g_tableRender.getTableScale())

// Formula to deduce the render offset for the second card of a holding, w is the visible width of the card after scale has been considered
#define HOLDING_R_OFFSET(w) ((float)(int)(w * 0.15625f))

class Player
{
	public:
		Player(uint8 seat, std::string name, unsigned int guid, float money, unsigned int avatar = 0);
		~Player();

		void Render();
		void RenderLabel();
		void RenderBet();
		void RenderClientCards();

		void ResetClientCards();
		void AddClientCard(ClientCard* pCard);

		void setBet(float bet) { m_bet = bet; }
		void setMoney(float money) { m_money = money; }
		void setAvatar(unsigned int avatar) { m_uiAvatar = avatar; }
		void setHolding(bool val) { m_uiInHand = val ? 2 : 0; }
		void setIsInHand(bool val) { m_bIsInHand = val; }

		bool isInHand() const { return m_bIsInHand; }

		float getBet() const { return m_bet; }
		
		unsigned int getGUID() const { return m_uiGUID; }
		unsigned int getAvatar() const { return m_uiAvatar; }

		uint8 getMySeat() const { return m_uiSeat; }

		Vector2 GetAnchor() const;

	private:	
		bool haveClientCards() const
		{
			for (uint8 i = 0; i < MAX_HOLD_CARDS; ++i)
				if (m_myCards[i])
					return true;
			return false;
		}

		uint8 m_uiSeat;
		uint8 m_uiInHand; // If m_myCards are empty and m_uiInHand is not null then we only render the cards face down

		bool m_bIsInHand;

		float m_bet;
		float m_money;

		unsigned int m_uiGUID;
		unsigned int m_uiAvatar;
		
		Vector2 m_anchor;

		std::string m_name;

		ClientCard* m_myCards[MAX_HOLD_CARDS];
};

#endif