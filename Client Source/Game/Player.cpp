#include "..\\stdafx.h"

// ----------------
// Constructor
Player::Player(uint8 seat, std::string name, unsigned int guid, float money, unsigned int avatar) :
	m_uiSeat(seat),
	m_name(name),
	m_uiGUID(guid),
	m_money(money),
	m_uiAvatar(avatar),
	m_bIsInHand(false),
	m_timeBank(0),
	m_decisionTime(0)
{
	for (unsigned int i = 0; i < MAX_HOLD_CARDS; ++i)
		m_myCards[i] = 0;

	m_uiInHand = 2;
	m_bet = 0.34f;
}

// ----------------
// Destructor
Player::~Player()
{
	m_name.clear();
}

// ---------------
// Render
//		This stuff is rendered before the player_frame is rendered
void Player::Render()
{
	// Fill cards from GameInfo
	//

	ResetClientCards();

	for (unsigned int i = 0; i < g_gameInfo.m_playerCards.size(); ++i)
		if (g_gameInfo.m_playerCards[i].guid == m_uiGUID)
			for (unsigned int c = 0; c < g_gameInfo.m_playerCards[i].cards.size(); ++c)
				AddClientCard(g_gameInfo.m_playerCards[i].cards[c]);

	// Update anchor location
	//

	m_anchor = PLAYER_RENDER_V(m_uiSeat);

	// Calculate some variables needed below
	//

	Vector2 origin(TABLE_CENTER_X, TABLE_CENTER_Y);
	float distToOrigin = Util::dist(origin, m_anchor);
			
	// Render Avatar
	//

	char spriteName[24];
	sprintf_s(spriteName, "avatar (%d).png", m_uiAvatar);

	hgeSprite* pAvatar = g_hgeClient.sprite(spriteName);

	if (!pAvatar)
		pAvatar = g_hgeClient.sprite("avatar (0).png");

	if (pAvatar)
	{
		CENTER_SPRITE_HOTSPOT(pAvatar);
		pAvatar->RenderEx(m_anchor.x, m_anchor.y, 0.0f, AVATAR_SCALE(pAvatar), AVATAR_SCALE(pAvatar));
	}

	// Render button if applicable
	//

	if (m_uiSeat == g_tableRender.getDealerSeat())
	{
		if (hgeSprite* pDealerButton = g_hgeClient.sprite("dealer_button.png"))
		{
			CENTER_SPRITE_HOTSPOT(pDealerButton);
			pDealerButton->RenderEx(D_BUTTON_RENDER_X(m_uiSeat), D_BUTTON_RENDER_Y(m_uiSeat), 0.0f, D_BUTTON_ICON_SCALE(pDealerButton), D_BUTTON_ICON_SCALE(pDealerButton));
		}
	}

	// Render holding
	//

	if (m_uiInHand && !haveClientCards())
	{
		if (hgeSprite* pHolding = g_hgeClient.sprite("holding.png"))
		{
			float scale = HOLDING_ICON_SCALE(pHolding);			
			float offset = HOLDING_R_OFFSET(pHolding->GetWidth() * scale);

			Vector2 renderPos = HOLDING_RENDER_V(m_uiSeat);

			CENTER_SPRITE_HOTSPOT(pHolding);

			for (uint8 i = 0; i < m_uiInHand; ++i)
				pHolding->RenderEx(renderPos.x + ((float)i * offset), renderPos.y - ((float)i * offset), 0, scale, scale);
		}
	}
}

// ------------------
// RenderLabel
//		Rendered after the player_frame is rendered
void Player::RenderLabel()
{
	Vector2 labelPos = PLAYER_LABEL_V(m_anchor.x, m_anchor.y);
	
	bool bIsMyTurn = g_gameInfo.m_uiSeatToAct == m_uiSeat;

	std::string labelSprite = bIsMyTurn && g_tableRender.playerFramePulse() ? "player_label_turn.png" : "player_label.png";

	if (hgeSprite* pPlayerLabel = g_hgeClient.sprite(labelSprite.c_str()))
	{
		CENTER_SPRITE_HOTSPOT(pPlayerLabel);
		pPlayerLabel->Render(labelPos.x, labelPos.y);

		if (hgeFont* pFont = g_hgeClient.font("tahoma13b.fnt"))
		{
			// Name
			//
			
			// Show timebank as "gummy52 (3)" (example)

			char nameStr[24];

			if (m_decisionTime <= 0 && m_uiInHand)
				sprintf_s(nameStr, "%s (%d)", m_name.c_str(), m_timeBank);
			else
				sprintf_s(nameStr, "%s", m_name.c_str());

			pFont->printf(Util::round(labelPos.x), Util::round(labelPos.y - 15.0f - (pFont->GetHeight() / 2.0f)), HGETEXT_CENTER, "%s", nameStr);

			// Money
			//

			pFont->printf(Util::round(labelPos.x), Util::round(labelPos.y + 15.0f - (pFont->GetHeight() / 2.0f)), HGETEXT_CENTER, "$%.2f", m_money);
		}
	}
}

// ----------------
// RenderClientCards
//		Rendered after the player_frame is rendered
void Player::RenderClientCards()
{
	float xOffset = 0.0f;

	for (uint8 i = 0; i < MAX_HOLD_CARDS; ++i)
	{
		if (m_myCards[i])
		{
			if (hgeSprite* pSprite = m_myCards[i]->Sprite())
			{
				CENTER_SPRITE_HOTSPOT(pSprite);
				
				pSprite->Render(PLAYER_RENDER_X(m_uiSeat) + xOffset, PLAYER_RENDER_Y(m_uiSeat));

				//float scale = CARD_RENDER_SCALE(pSprite);
				//pSprite->RenderEx(PLAYER_RENDER_X(m_uiSeat) + xOffset, PLAYER_RENDER_Y(m_uiSeat), 0.0f, scale, scale);
				//xOffset += (pSprite->GetWidth() * scale) * 0.5f;

				xOffset += floor(pSprite->GetWidth() * 0.5f);
			}
		}
	}
}

// ------------------
// RenderBet
//		Rendered after RenderLabel
void Player::RenderBet()
{
	if (m_bet >= 0.01f)
		g_tableRender.RenderChipStack(m_bet, m_uiSeat);
}

// ------------------
// AddClientCard
void Player::AddClientCard(ClientCard* pCard)
{
	for (uint8 i = 0; i < MAX_HOLD_CARDS; ++i)
	{
		if (!m_myCards[i])
		{
			m_myCards[i] = pCard;
			return;
		}
	}

	printf("ERROR: Failed to addClientCard!\n");
}

// --------------------
// ResetClientCards
void Player::ResetClientCards()
{
	// No need to delete because they're perma in CardLibrary
	for (uint8 i = 0; i < MAX_HOLD_CARDS; ++i)
		m_myCards[i] = 0;
}

// ----------------------
// GetAnchor
Vector2 Player::GetAnchor() const
{ 
	return PLAYER_RENDER_V(m_uiSeat);
}