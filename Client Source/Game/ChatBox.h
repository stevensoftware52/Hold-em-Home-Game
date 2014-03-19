#ifndef _CHATBOX_H
#define _CHATBOX_H

#include "stdafx.h"

// Hotspot for chatbox sprite is bottom left
#define CHATBOX_RENDER_X 0.0f
#define CHATBOX_RENDER_Y SCREEN_HEIGHT_F

// Max num messages printed at once
#define CHATBOX_MAX_PRINTED_MSG 7

enum ChatTypes
{
	CHATTYPE_NULL,
	CHATTYPE_SERVER_MSG,
	CHATTYPE_PLAYER_MSG,
	CHATTYPE_DIRECT_SERVER_MSG,
};

struct ChatMsg
{
	ChatMsg(uint8 t, std::string m) : 
		type(t), msg(m) {}

	uint8 type;
	std::string msg;
};

enum eChatBoxButtons
{
	CBUTTON_SCROLL_UP,
	CBUTTON_SCROLL_DOWN,
	CBUTTON_SCROLL_DOWNX,
	NUM_CHATBOX_BUTTONS
};

class ChatBox : public InputBox
{
	public:
		ChatBox();
		~ChatBox();

		void Input();
		void Render();
		void PressedEnter();

		void pushMsg(uint8 type, std::string msg) { m_messages.push_back(ChatMsg(type, msg)); }

		bool CursorInBounds() const;

	private:
		void ColorizedPrint(hgeFont* pFont, float x, float y, ChatMsg msg);

		bool m_bInUse;

		float m_boxHeight;
		float m_boxWidth;

		unsigned int m_uiScrollOffset;

		Button* m_buttons[NUM_CHATBOX_BUTTONS];

		std::vector<ChatMsg> m_messages;
};

#endif