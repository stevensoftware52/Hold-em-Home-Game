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
	
	DWORD timeBank = 0;
	DWORD decisionTime = 0;

	if (Player* pPlayer = g_tableRender.GetPlayer(this->m_uiGUID))
	{
		timeBank = pPlayer->m_timeBank;
		decisionTime = pPlayer->m_decisionTime;
	}

	if (hgeFont* pFont = g_hgeClient.font("tahoma13b"))
		pFont->printf(SCREEN_WIDTH_F - 15.0f, 15.0f, HGETEXT_RIGHT, "Username: %s\nNet Bankroll: $%.2f\nAvatar: %d\nDecision Time: %d\nTime Bank: %d", 
			m_username.c_str(), m_fBankroll, myAvatar, decisionTime, timeBank);
}

// ------------------------
// PumpAnimations
void GameInfo::PumpAnimations()
{
	// Card animations
	//

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

	// Winner animations
	//

	{
		auto itr = m_winnerAnimations.begin();

		while (itr != m_winnerAnimations.end())
		{
			// Play chip sound once it starts moving

			bool bPlaySound = (*itr).progress < 0;

			(*itr).progress += _hge->Timer_GetDelta() * 2.0f;
			
			if (bPlaySound && (*itr).progress >= 0)
				g_hgeClient.ToggleSound("pot.wav", true);

			// Done at > 1.0f

			if ((*itr).progress > 4.0f)
			{
				itr = m_winnerAnimations.erase(itr);
			}
			
			// Render

			else
			{
				WinnerAnimation wA = *itr;
				
				// Sit in the middle until progress >= 0

				if (wA.progress < 0)
					wA.progress = 0;

				// Sit at player's ring until object is erased

				if (wA.progress > 1.0f)
					wA.progress = 1.0f;

				// Render based on percent distance to endPos
				{
					if (Player* pPlayer = g_tableRender.GetPlayer(wA.guid))
					{
						Vector2 startPos(TABLE_CENTER_X, TABLE_CENTER_Y);
						Vector2 endPos(BET_RENDER_V(pPlayer->getMySeat()));

						float distFromOrigin = Util::dist(startPos, endPos);

						// Our visual position is extruded from origin to endPos
						Vector2 renderPos = Util::extrude(startPos.x, startPos.y, endPos.x, endPos.y, distFromOrigin * wA.progress);

						// Render chips
						g_tableRender.RenderChipStack(wA.fAmount, 0, renderPos);
					}
				}

				++itr;
			}
		}
}
}