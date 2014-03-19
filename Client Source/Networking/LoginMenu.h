#ifndef _LOGINMENU_H
#define _LOGINMENU_H

#include "stdafx.h"

// We expect login_menu.png to be 314 x 285 static size

// Location of Username inputbox
#define USERNAME_INPUT_X (SCREEN_C_X - 61.0f)
#define USERNAME_INPUT_Y (SCREEN_C_Y - 44.0f)

// Location of Password inputbox
#define PASSWORD_INPUT_X (SCREEN_C_X - 61.0f)
#define PASSWORD_INPUT_Y (SCREEN_C_Y - 4.00f)

// Max width of inputbox string
#define LOGINMENU_MAX_IB_STR 140.0f

// The font the login menu uses
#define LOGINMENU_FONT "tahoma13.fnt"

// Login button Vector2's
#define LOGIN_BUTTON_VA Vector2(SCREEN_C_X - 29.0f, SCREEN_C_Y + 41.0f)
#define LOGIN_BUTTON_VB Vector2(SCREEN_C_X + 31.0f, SCREEN_C_Y + 68.0f)

class Button;

class LoginMenu : InputBox
{
	public:
		LoginMenu();
		~LoginMenu();

		void Input();
		void Render();
		void PressedEnter();

	private:
		InputBox* m_passwordBox;
		Button* m_loginButton;
};

#endif