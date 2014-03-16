#ifndef _TABLERENDER_H
#define _TABLERENDER_H

#include "stdafx.h"

// How big the table is relative to screen width
#define TABLE_RELATIVE_SCALE 0.6791666f

// How big the table sprite should be
#define TABLE_RENDER_SCALE(a) ((SCREEN_WIDTH_F / a->GetWidth()) * TABLE_RELATIVE_SCALE)

// Where the center of the table should be at
#define TABLE_CENTER_X SCREEN_C_X
#define TABLE_CENTER_Y (SCREEN_C_Y - (SCREEN_HEIGHT * 0.1f))

#define MAX_PLAYERS 6

// Relative render positions
static float g_playerPosRelative[MAX_PLAYERS][2] =
{
	// { CENTER_X + (Width * VALUE) } , { CENTER_Y + (Height * VALUE) }
			
		{  0.50f, 0.0f,	},	// Right
		{  0.25f, 0.55f },	// Bottom Right
		{ -0.25f, 0.55f },	// Bottom Left	
		{ -0.50f, 0.0f },	// Left	
		{ -0.25f, -0.63f }, // Top Left		
		{  0.25f, -0.63f }, // Top Right	
};

// Formula to take g_playerPosRelative and get a render position. (a) is the seat number, which should not be >= MAX_PLAYERS
#define PLAYER_RENDER_X(a) (float)(int)(TABLE_CENTER_X + (g_tableRender.getTableWidth() * g_playerPosRelative[a][0]))
#define PLAYER_RENDER_Y(a) (float)(int)(TABLE_CENTER_Y + (g_tableRender.getTableHeight() * g_playerPosRelative[a][1]))
#define PLAYER_RENDER_V(a) Vector2(PLAYER_RENDER_X(a), PLAYER_RENDER_Y(a))

// Scale of the player frame sprite
#define PLAYERFRAME_SCALE (g_tableRender.getTableScale() * 1.7734375f)

// Formula to decide scale of avatar
#define AVATAR_SCALE(a) ((g_tableRender.getPlayerFrameSize() / a->GetWidth()) * 0.8f)

// Max number of chips to a colum of a stack (when visually rendering a stack)
#define MAX_CHIP_C_STACK 4

// The value of a chip matched with the name of the corrosponding texture
//

#define MAX_CHIP_TYPES 5

struct Chip
{
	float value;
	std::string name;
};

static Chip g_chipValueTexture[MAX_CHIP_TYPES] = 
{
	{ 0.01f, "penny.png" },
	{ 0.05f, "nickle.png" },
	{ 0.25f, "quarter.png" },
	{ 1.00f, "dollar.png" },
	{ 5.00f, "five dollar.png" }
};

// Formula to decide scale of a chip
#define CHIP_SCALE(a) ((64.0f / a->GetWidth()) * g_tableRender.getTableScale())

// Relative render positions of a player's bet
static float g_betPosRelative[MAX_PLAYERS][2] = 
{
	// { CENTER_X + (Width * VALUE) } , { CENTER_Y + (Height * VALUE) }
			
		{  0.30f, 0.00f },  // Right
		{  0.22f, 0.22f },  // Bottom Right	
		{ -0.22f, 0.22f },  // Bottom Left	
		{ -0.30f, 0.00f },  // Left
		{ -0.22f, -0.22f }, // Top Left		
		{  0.22f, -0.22f }, // Top Right
};

// Formula to take g_holdingPosRelative and get a render position. (a) is the seat number, which should not be >= MAX_PLAYERS
#define BET_RENDER_X(a) (float)(int)(TABLE_CENTER_X + (g_tableRender.getTableWidth() * g_betPosRelative[a][0]))
#define BET_RENDER_Y(a) (float)(int)(TABLE_CENTER_Y + (g_tableRender.getTableHeight() * g_betPosRelative[a][1]))
#define BET_RENDER_V(a) Vector2(BET_RENDER_X(a), BET_RENDER_Y(a))

// How far up you have to move to render a chip on top of another chip
#define BET_CHIP_Y_SCALE 0.15f // This percentage of the sprite's size

// Relative position of where the pot is rendered
static float g_posPosRelative[2] = { 0.0f, -0.15f };

// Formula to take g_posPosRelative and get a render position. (a) is the seat number, which should not be >= MAX_PLAYERS
#define POT_RENDER_X (float)(int)(TABLE_CENTER_X + (g_tableRender.getTableWidth() * g_posPosRelative[0]))
#define POT_RENDER_Y (float)(int)(TABLE_CENTER_Y + (g_tableRender.getTableHeight() * g_posPosRelative[1]))
#define POT_RENDER_V Vector2(POT_RENDER_X, POT_RENDER_Y)

// Max cards on board
#define MAX_CARDS_BOARD 5

// Buttons
//

#define NUM_BUTTONS 7

static const char* g_buttonTitles[NUM_BUTTONS] = 
{
	"Sit Out",
	"Sit Down",
	"All In",
	"Bet/Raise",
	"Call",
	"Check",
	"Fold",
};

// Anchor position of buttons
#define BUTTON_ANCHOR Vector2(SCREEN_WIDTH_F - Util::round(BUTTON_SPRITE_SIZE * 0.75f), SCREEN_HEIGHT_F - Util::round(BUTTON_SPRITE_SIZE * 0.75f))

// Where the amount input box is rendered (based on the 177 x 48 texture)
#define A_INPUTBOX_X (SCREEN_WIDTH_F - 150.0f)
#define A_INPUTBOX_Y (SCREEN_HEIGHT_F - 140.0f)

// How big the music button is
#define MUSIC_BUTTON_SIZE 34.0f

// Where the button to toggle music is 
#define MUSIC_BUTTON_ANCHORA Vector2(15.0f, 15.0f)
#define MUSIC_BUTTON_ANCHORB Vector2(15.0f + MUSIC_BUTTON_SIZE, 15.0f + MUSIC_BUTTON_SIZE)
#define MUSIC_BUTTON_CENTER Vector2(15.0f + (MUSIC_BUTTON_SIZE / 2.0f), 15.0f + (MUSIC_BUTTON_SIZE / 2.0f))

// How many seconds to toggle the player who's turn it is rendering as yellow back/forth
#define PLAYER_TURN_FLASH_RATE 1.0f

class Player;
class Button;
class Slider;

class TableRender : InputBox
{
	public:
		TableRender();
		~TableRender();

		void Render();
		void InterfaceInput();
		void AddPlayer(uint8 seat, Player* pPlayer);
		void RemovePlayer(unsigned int guid);
		void RemoveAllPlayers();
		void ResetBoard();
		void AddBoardCard(ClientCard* pCard);
		void ToggleMusic(bool bOn);
		void RenderChipStack(float amount, uint8 seat);

		void setPot(float pot) { m_pot = pot; }
		void setDealerSeat(uint8 seat) { m_dealerSeat = seat; }
		void setMsg(std::string msg) { m_typedMsg = msg; }

		bool playerFramePulse() const { return m_bPlayerFramePulse; }

		float NetPot() const;
		float getTableWidth() const { return m_tableWidth; }
		float getTableHeight() const { return m_tableHeight; }
		float getPlayerFrameSize() const { return m_playerFrameSize; }
		float getTableScale() const { return m_tableScale; }
		float getTypedAmount() const { return (float)atof(m_typedMsg.c_str()); }

		uint8 getDealerSeat() const { return m_dealerSeat; }


		Player* GetPlayer(unsigned int guid) const;

		std::vector<Player*> getPlayers() const
		{
			std::vector<Player*> result;

			for (uint8 i = 0; i < MAX_PLAYERS; ++i)
				if (m_players[i])
					result.push_back(m_players[i]);

			return result;
		}

		std::vector<ClientCard*> getBoard() const { return m_board; }

	private:
		bool IsSomethingToInput() const;

		float m_tableX;
		float m_tableY;
		float m_tableHeight;
		float m_tableWidth;
		float m_tableScale;
		float m_playerFrameSize;
		float m_fPlayerFramePulse;

		bool m_bPlayerFramePulse;

		float m_pot;		// How much is in the center

		uint8 m_dealerSeat;

		bool m_bMusic;

		Player* m_players[MAX_PLAYERS];

		Button* m_musicButton;
		Button* m_buttons[NUM_BUTTONS];

		Slider* m_slider;

		std::vector<ClientCard*> m_board;
};

#endif