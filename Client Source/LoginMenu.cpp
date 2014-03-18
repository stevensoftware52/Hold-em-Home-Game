#include "stdafx.h"

// -------------------
// Constructor
LoginMenu::LoginMenu() :
	InputBox(USERNAME_INPUT_X, USERNAME_INPUT_Y, HGETEXT_LEFT, LOGINMENU_MAX_IB_STR, LOGINMENU_FONT, true, 0xFF000000)
{
	m_loginButton = new Button(Vector2(LOGIN_BUTTON_VA), Vector2(LOGIN_BUTTON_VB));
	m_passwordBox = new InputBox(PASSWORD_INPUT_X, PASSWORD_INPUT_Y, HGETEXT_LEFT, LOGINMENU_MAX_IB_STR, LOGINMENU_FONT, true, 0xFF000000);
}

// --------------------
// Destructor
LoginMenu::~LoginMenu()
{
	delete m_passwordBox;
	delete m_loginButton;
}

// -------------------
// Input
void LoginMenu::Input()
{
	// Have tab switch between the two
	if (_hge->Input_GetKey() == HGEK_TAB)
	{
		if (InputBox::InUse())
			g_hgeClient.changeInputBox(m_passwordBox);
		else if (m_passwordBox->InUse())
			g_hgeClient.changeInputBox(this);
	}
	else
	{
		InputBox::FocusLogic();
		m_passwordBox->FocusLogic();
	}

	// If the login button is pressed
	if (m_loginButton->ButtonPressed() || _hge->Input_GetKey() == HGEK_ENTER)
	{
		PressedEnter();
	}
}

// -----------------
// Render
void LoginMenu::Render()
{
	// Background
	//

	if (hgeSprite* pBackground = g_hgeClient.sprite("login_background.png"))
		pBackground->RenderStretch(0, 0, SCREEN_WIDTH_F, SCREEN_HEIGHT_F);

	// Login Menu
	//

	if (hgeSprite* pLoginMenu = g_hgeClient.sprite("login_menu.png"))
	{
		CENTER_SPRITE_HOTSPOT(pLoginMenu);
		pLoginMenu->Render(floor(SCREEN_C_X), floor(SCREEN_C_Y));
	}


	// Inputbox's
	//

	// Make the font black before rendering
	if (hgeFont* pFont = g_hgeClient.font(LOGINMENU_FONT))
	{
		pFont->SetColor(0xFF000000);

		// Username
		InputBox::Render();

		// Password
		m_passwordBox->Render();

		pFont->SetColor(0xFFFFFFFF);
	}
}

// ---------------------
// PressedEnter
void LoginMenu::PressedEnter()
{
	if (InputBox::m_typedMsg.size() && m_passwordBox->m_typedMsg.size())
	{
		g_tcpClient.AttemptConnection(InputBox::m_typedMsg.c_str(), m_passwordBox->m_typedMsg.c_str());
			
		if (g_tcpClient.m_bConnected)
		{
			InputBox::m_typedMsg.clear();
			m_passwordBox->m_typedMsg.clear();
			g_hgeClient.changeInputBox(0);
		}
	}
	else
		printf("Please enter a username & password\n");
}