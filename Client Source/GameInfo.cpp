#include "stdafx.h"

// -----------------
// Update
void GameInfo::Update()
{
	// Timedown player taunts
	//

	auto itr = m_playerTaunts.begin();

	while (itr != m_playerTaunts.end())
	{
		(*itr).durationLeft -= _hge->Timer_GetDelta();

		if ((*itr).durationLeft < 0)
		{
			itr = m_playerTaunts.erase(itr);
		}
		else
			++itr;
	}
}

// --------------------
// RenderText
void GameInfo::RenderText()
{
	uint8 myAvatar = 0;

	if (Player* pPlayer = g_tableRender.GetPlayer(this->m_uiGUID))
		myAvatar = pPlayer->getAvatar();

	if (hgeFont* pFont = g_hgeClient.font("tahoma13b"))
		pFont->printf(SCREEN_WIDTH_F - 15.0f, 15.0f, HGETEXT_RIGHT, "Username: %s\nNet Bankroll: $%.2f\nAvatar: %d", m_username.c_str(), m_fBankroll, myAvatar);
}

// ------------------------
// PumpDrawCardAnimation
void GameInfo::PumpDrawCardAnimation()
{
	auto itr = m_cardAnimations.vCardAnimations.begin();

	while (itr != m_cardAnimations.vCardAnimations.end())
	{
		(*itr).progression += _hge->Timer_GetDelta() * 3.0f;

		if ((*itr).progression > 1.0f)
		{
			itr = m_cardAnimations.vCardAnimations.erase(itr);
		}
		else
		{
			CardAnimation cA = (*itr);

			// Render based on percent distance to endPos
			if (cA.progression >= 0.0f)
			{
				float distFromOrigin = Util::dist(cA.startPos, cA.endPos);

				// Our visual position is extruded from origin to endPos
				Vector2 renderPos = Util::extrude(cA.startPos.x, cA.startPos.y, cA.endPos.x, cA.endPos.y, distFromOrigin * cA.progression);
				
				// Render it
				if (hgeSprite* pSprite = g_hgeClient.sprite("holding.png"))
				{
					float scale = HOLDING_ICON_SCALE(pSprite);	
					CENTER_SPRITE_HOTSPOT(pSprite);
					pSprite->RenderEx(renderPos.x, renderPos.y, 0.0f, scale, scale);
				}
			}

			++itr;
		}
	}
}