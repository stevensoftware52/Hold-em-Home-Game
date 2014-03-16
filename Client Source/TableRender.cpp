#include "stdafx.h"

// ------------------
// Constructor
TableRender::TableRender() : InputBox(A_INPUTBOX_X - 60.0f, A_INPUTBOX_Y, HGETEXT_LEFT, 100.0f, "tahoma13.fnt", STRICT_MONEY, 0xFF000000),
	m_fPlayerFramePulse(0),
	m_bPlayerFramePulse(0),
	m_bMusic(false),
	m_dealerSeat(2),
	m_pot(0)
{
	m_typedMsg = "5.00";

	m_musicButton = new Button(MUSIC_BUTTON_ANCHORA, MUSIC_BUTTON_ANCHORB);

	for (unsigned int i = 0; i < NUM_BUTTONS; ++i)
		m_buttons[i] = new Button(BUTTON_ANCHOR, g_buttonTitles[i]);

	for (unsigned int i = 1; i < MAX_PLAYERS; ++i)
		m_players[i] = 0;
}

// ------------------------
// Destructor
TableRender::~TableRender()
{
	m_board.clear();
	
	delete m_musicButton;

	for (unsigned int i = 0; i < NUM_BUTTONS; ++i)
		delete m_buttons[i];
}

// -----------------------
// Render
void TableRender::Render()
{
	// Background
	//

	if (hgeSprite* pBackground = g_hgeClient.sprite("background.png"))
		pBackground->RenderStretch(0, 0, SCREEN_WIDTH_F, SCREEN_HEIGHT_F);

	// Table
	//

	if (hgeSprite* pTable = g_hgeClient.sprite("table.png"))
	{
		m_tableX = TABLE_CENTER_X;
		m_tableY = TABLE_CENTER_Y;
		m_tableWidth = pTable->GetWidth() * TABLE_RENDER_SCALE(pTable);
		m_tableHeight = pTable->GetHeight() * TABLE_RENDER_SCALE(pTable);
		m_tableScale = TABLE_RENDER_SCALE(pTable);

		CENTER_SPRITE_HOTSPOT(pTable);
		pTable->RenderEx(m_tableX, m_tableY, 0.0f, m_tableScale, m_tableScale);
	}

	// Board
	//

	// Use the GameInfo image of the board if an animation is in progress
	if (g_gameInfo.m_cardAnimations.vCardAnimations.size())
		m_board = g_gameInfo.m_cardAnimations.vBoard;

	for (unsigned int i = 0; i < m_board.size(); ++i)
	{
		if (hgeSprite* pSprite = m_board[i]->Sprite())
		{
			CENTER_SPRITE_HOTSPOT(pSprite);

			float cardSize = CARD_RENDERED_SIZE(pSprite);			
			float renderX = TABLE_CENTER_X - ((cardSize / 2.0f) * m_board.size()) + (cardSize * i) + (cardSize / 2.0f);
			float renderY = TABLE_CENTER_Y;

			pSprite->RenderEx(renderX, renderY, 0.0f, CARD_RENDER_SCALE(pSprite), CARD_RENDER_SCALE(pSprite));
		}
	}

	// Players
	//
	
	m_fPlayerFramePulse += _hge->Timer_GetDelta();

	if (m_fPlayerFramePulse >= PLAYER_TURN_FLASH_RATE)
	{
		m_bPlayerFramePulse = !m_bPlayerFramePulse;
		m_fPlayerFramePulse = 0;	
	}

	if (hgeSprite* pPlayerFrame = g_hgeClient.sprite("player_frame.png"))
	{
		for (uint8 i = 0; i < MAX_PLAYERS; ++i)
		{
			if (Player* pPlayer = m_players[i])
			{
				float renderX = PLAYER_RENDER_X(i);
				float renderY = PLAYER_RENDER_Y(i);

				m_playerFrameSize = (pPlayerFrame->GetWidth() * PLAYERFRAME_SCALE);
			
				CENTER_SPRITE_HOTSPOT(pPlayerFrame);

				pPlayer->Render();
				pPlayerFrame->RenderEx(renderX, renderY, 0.0f, PLAYERFRAME_SCALE, PLAYERFRAME_SCALE);
				pPlayer->RenderLabel();				
				pPlayer->RenderBet();
				pPlayer->RenderClientCards();
								
				// Render a speaker icon over his head if they're taunting
				if (g_gameInfo.isTaunting(pPlayer->getGUID()))
				{
					if (hgeSprite* pSprite = g_hgeClient.sprite("sound_icon.png"))
					{
						CENTER_SPRITE_HOTSPOT(pSprite);
						pSprite->Render(pPlayer->GetAnchor().x, pPlayer->GetAnchor().y);
					}
				}
			}
		}
	}

	// Pot
	//
	
	if (m_pot || NetPot())
		RenderChipStack(m_pot, MAX_PLAYERS);

	// Interface 
	//
		
	Vector2 buttonPos = BUTTON_ANCHOR;
		
	for (unsigned int i = 0; i < NUM_BUTTONS; ++i)
	{
		if (!g_gameInfo.buttonIsOption(g_buttonTitles[i]))
		{			
			m_buttons[i]->setPosition(Vector2(-SCREEN_WIDTH_F, -SCREEN_HEIGHT_F));
			continue;
		}

		m_buttons[i]->setPosition(buttonPos);
		m_buttons[i]->Render();

		buttonPos.x -= BUTTON_SPRITE_SIZE;
	}

	if (IsSomethingToInput())
	{
		if (hgeSprite* pSprite = g_hgeClient.sprite("text_box.png"))
		{
			CENTER_SPRITE_HOTSPOT(pSprite);
			pSprite->Render(A_INPUTBOX_X, A_INPUTBOX_Y);
		}
	}

	// Input Box
	//

	if (IsSomethingToInput())
		InputBox::Render();

	// Music button
	//

	if (hgeSprite* pSprite = g_hgeClient.sprite("music_icon"))
	{
		if (m_bMusic)
			pSprite->SetColor(0xFFFFFFFF);
		else
			pSprite->SetColor(0x99FFFFFF);

		CENTER_SPRITE_HOTSPOT(pSprite);
		pSprite->Render(MUSIC_BUTTON_CENTER.x, MUSIC_BUTTON_CENTER.y);
	}
}

// --------------------
// InterfaceInput
void TableRender::InterfaceInput()
{	
	for (unsigned int i = 0; i < NUM_BUTTONS; ++i)
	{
		if (m_buttons[i]->ButtonPressed())
			g_tcpClient.SendButtonPress(g_buttonTitles[i]);
	}

	if (m_musicButton->ButtonPressed())
	{
		ToggleMusic(!m_bMusic);
	}	

	if (IsSomethingToInput())
		InputBox::FocusLogic();
}

// --------------------
// ToggleMusic
void TableRender::ToggleMusic(bool bOn)
{
	m_bMusic = bOn;
	g_hgeClient.ToggleSound("music.wav", bOn, 15);
}

// ----------------------
// AddPlayer
void TableRender::AddPlayer(uint8 seat, Player* pPlayer)
{
	if (seat < MAX_PLAYERS)
	{
		if (m_players[seat])
		{
			printf("ERROR: Attempted to add a player to seat %d, but someone is already there!\n");
			assert(0);
			return;
		}

		m_players[seat] = pPlayer;
	}
}

// ---------------------
// RemovePlayer
void TableRender::RemovePlayer(unsigned int guid)
{
	for (uint8 i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_players[i] && m_players[i]->getGUID() == guid)
		{
			delete m_players[i];
			m_players[i] = 0;
		}
	}
}

// ---------------------
// RemoveAllPlayers
void TableRender::RemoveAllPlayers()
{
	for (uint8 i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_players[i])
		{
			delete m_players[i];
			m_players[i] = 0;
		}
	}
}

// -----------------
// ResetBoard
void TableRender::ResetBoard()
{
	m_board.clear();
}

// ------------
// NetPot
float TableRender::NetPot() const
{
	float result = 0;

	for (uint8 i = 0; i < MAX_PLAYERS; ++i)
		if (m_players[i] && m_players[i]->getBet())
			result += m_players[i]->getBet();

	return result + m_pot;
}

// -----------------
// RenderChipStack
//		If seat is >= MAX_PLAYERS then it's rendering the pot
void TableRender::RenderChipStack(float amount, uint8 seat)
{
	bool bIsThePot = seat >= MAX_PLAYERS;
	float betAmount = amount;

	std::vector<Chip> chips;

	// Grab from the biggest chip until we can't grab that anymore, then go down in size, repeat, etc
	while (amount >= 0.01f)
	{
		for (unsigned int i = MAX_CHIP_TYPES - 1; i >= 0; i--)
		{
			if (amount >= g_chipValueTexture[i].value)
			{
				chips.push_back(g_chipValueTexture[i]);
				amount -= g_chipValueTexture[i].value;
				break;
			}
		}
	}

	// Origin & Render position
	//
	
	Vector2 originPos;

	if (bIsThePot)
		originPos = POT_RENDER_V;
	else
		originPos = BET_RENDER_V(seat);

	Vector2 renderPos = originPos;
	
	float chipVisualSize = 0.0f;

	// Whether or not to add or subtract from X/Y position
	//

	bool bXAdd = false;
	bool bYSub = false;
	
	if (originPos.x < TABLE_CENTER_X)
		bXAdd = true;

	if (!bIsThePot && originPos.y > TABLE_CENTER_Y)
		bYSub = true;

	// Render that turky
	//
	
	for (unsigned int i = 0, c = 0; i < chips.size(); ++i, ++c)
	{
		if (hgeSprite* pChip = g_hgeClient.sprite(chips[i].name.c_str()))
		{
			if (c > MAX_CHIP_C_STACK)
			{
				c = 0;

				if (bXAdd)
					renderPos.x += pChip->GetWidth() * CHIP_SCALE(pChip);
				else
					renderPos.x -= pChip->GetWidth() * CHIP_SCALE(pChip);

				renderPos.y = originPos.y;
			}

			CENTER_SPRITE_HOTSPOT(pChip);
			chipVisualSize = (pChip->GetWidth() * CHIP_SCALE(pChip));
			pChip->RenderEx(renderPos.x, renderPos.y, 0.0f, CHIP_SCALE(pChip), CHIP_SCALE(pChip));
			renderPos.y -= chipVisualSize * BET_CHIP_Y_SCALE;
		}
	}

	// We don't the Y axis of printing to change based on how many chips there are
	// But we DO want the text to move on over
	float printX = renderPos.x;
	float printY = originPos.y;

	// If its the pot, though...
	if (bIsThePot)
	{
		printX = originPos.x;
		printY = originPos.y - ((chipVisualSize * BET_CHIP_Y_SCALE) * MAX_CHIP_C_STACK * 4.0f); // * 1.25;

		betAmount = NetPot();
	}

	// Otherwise, rRender the amount next to it
	else
	{		
		if (bXAdd)
			printX += chipVisualSize * 0.75f;
		else
			printX -= chipVisualSize * 0.75f;
	}

	if (hgeFont* pFont = g_hgeClient.font("tahoma13b.fnt"))
	{
		if (bYSub)
			printY -= pFont->GetHeight() * 2.0f;

		pFont->printf(Util::round(printX), Util::round(printY), bIsThePot ? HGETEXT_CENTER : bXAdd ? HGETEXT_LEFT : HGETEXT_RIGHT, "$%.2f", betAmount);
	}
}

// --------------
// AddBoardCard
void TableRender::AddBoardCard(ClientCard* pCard)
{
	if (m_board.size() >= MAX_CARDS_BOARD)
	{
		printf("ERROR: Board is full, but told to add a card.\n");
		return;
	}

	if (pCard)
		m_board.push_back(pCard);
}

// --------------
// GetPlayer
Player* TableRender::GetPlayer(unsigned int guid) const
{
	for (uint8 i = 0; i < MAX_PLAYERS; ++i)
		if (m_players[i] && m_players[i]->getGUID() == guid)
			return m_players[i];
	return 0;
}

// ---------------------
// IsSomethingToInput
bool TableRender::IsSomethingToInput() const
{
	bool bIsMyTurn = false;

	if (Player* pPlayer = GetPlayer(g_gameInfo.m_uiGUID))
		bIsMyTurn = pPlayer->isInHand() && pPlayer->getMySeat() == g_gameInfo.m_uiSeatToAct;

	// We only have something to input if its our turn or we need to enter the amount to sit down for
	return bIsMyTurn || find(g_gameInfo.m_buttonOptions.begin(), g_gameInfo.m_buttonOptions.end(), "Sit Down") != g_gameInfo.m_buttonOptions.end();
}