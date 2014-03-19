#include "..\\stdafx.h"

// --------------
// Constructor
ChatBox::ChatBox() : 
	InputBox(CHATBOX_RENDER_X + 35.0f, CHATBOX_RENDER_Y - 15.0f, HGETEXT_LEFT)
{
	m_boxWidth = 0;
	m_boxHeight = 0;
	m_uiScrollOffset = 0;
			
	m_buttons[CBUTTON_SCROLL_UP] = new Button(Vector2(CHATBOX_RENDER_X, CHATBOX_RENDER_Y - 148.0f), Vector2(CHATBOX_RENDER_X + 27.0f, CHATBOX_RENDER_Y - 123.0f));
	m_buttons[CBUTTON_SCROLL_DOWN] = new Button(Vector2(CHATBOX_RENDER_X, CHATBOX_RENDER_Y - 120.0f), Vector2(CHATBOX_RENDER_X + 27.0f, CHATBOX_RENDER_Y - 96.0f));
	m_buttons[CBUTTON_SCROLL_DOWNX] = new Button(Vector2(CHATBOX_RENDER_X, CHATBOX_RENDER_Y - 55.0f), Vector2(CHATBOX_RENDER_X + 27.0f, CHATBOX_RENDER_Y - 30.0f));
}

// -----------------
// Destructor
ChatBox::~ChatBox()
{
	for (uint8 i = 0; i < NUM_CHATBOX_BUTTONS; ++i)
		delete m_buttons[i];
}

// -----------------
// Input
void ChatBox::Input()
{
	// Scroll up button
	if ((_hge->Input_GetMouseWheel() == 1 || m_buttons[CBUTTON_SCROLL_UP]->ButtonPressed()) && m_messages.size() > CHATBOX_MAX_PRINTED_MSG)
		m_uiScrollOffset = min(m_uiScrollOffset + 1, m_messages.size() - CHATBOX_MAX_PRINTED_MSG);
	
	// Scroll down button
	if (( _hge->Input_GetMouseWheel() == -1 || m_buttons[CBUTTON_SCROLL_DOWN]->ButtonPressed()) && m_uiScrollOffset >= 1)
		--m_uiScrollOffset;
	
	// Scroll down "all the way" button   
	if (m_buttons[CBUTTON_SCROLL_DOWNX]->ButtonPressed())
		m_uiScrollOffset = 0;

	// If click on chat box, it's now inputting
	if (CursorInBounds() && _hge->Input_GetKey() == HGEK_LBUTTON)
		g_hgeClient.changeInputBox(this);

	// If press enter, now inputting
	if (_hge->Input_GetKey() == HGEK_ENTER)
		g_hgeClient.changeInputBox(this);
}

// ----------
// Render
void ChatBox::Render()
{
	// Chatbox
	//

	if (hgeSprite* pChatbox = g_hgeClient.sprite("chatbox.png"))
	{
		setMaxLength(pChatbox->GetWidth());

		m_boxWidth = pChatbox->GetWidth();
		m_boxHeight = pChatbox->GetHeight();

		// Color if mousing over only
		if (!InUse() && CursorInBounds())
			pChatbox->SetColor(0xDEFFFFFF);

		// Color if in use
		else if (InUse())
			pChatbox->SetColor(0xFFFFFFFF);

		// Color if not being used
		else
			pChatbox->SetColor(0xAAFFFFFF);

		// I choose not to scale the chatbox based on window dimensions
		pChatbox->SetHotSpot(0.0f, pChatbox->GetHeight());
		pChatbox->Render(CHATBOX_RENDER_X, CHATBOX_RENDER_Y);
	}

	// Chat msgs
	//

	if (hgeFont* pFont = g_hgeClient.font("tahoma13.fnt"))
	{
		Vector2 renderPos(InputBox::m_renderX, InputBox::m_renderY - pFont->GetHeight());

		uint8 count = 0;

		for (int i = m_messages.size() - 1 - m_uiScrollOffset; i >= 0; --i)
		{
			renderPos.y -= pFont->GetHeight();
			ColorizedPrint(pFont, Util::round(renderPos.x), (float)(int)renderPos.y, m_messages[i]);

			++count;

			if (count >= CHATBOX_MAX_PRINTED_MSG)
				break;
		}
	}

	// Render the input box manually even if its not in use
	if (!InUse())
		InputBox::Render();
}


// ---------------
// CursorInBounds
bool ChatBox::CursorInBounds() const
{
	return (_input.x > CHATBOX_RENDER_X && _input.x < CHATBOX_RENDER_X + m_boxWidth &&
		_input.y < CHATBOX_RENDER_Y && _input.y > CHATBOX_RENDER_Y - m_boxHeight);
}

// ----------------------
// ColorizedPrint
void ChatBox::ColorizedPrint(hgeFont* pFont, float x, float y, ChatMsg msg)
{
	DWORD color = 0xFFFFFFFF;

	switch (msg.type)
	{
		case CHATTYPE_PLAYER_MSG:				color = 0xFFFFFFFF; break;
		case CHATTYPE_SERVER_MSG:				color = 0xFFFFCC11; break;
		case CHATTYPE_DIRECT_SERVER_MSG:		color = 0xFF00FF00; break;
	}

	pFont->SetColor(color);
	pFont->printf(x, y, HGETEXT_LEFT, "%s", msg.msg.c_str());
	pFont->SetColor(0xFFFFFFFF);
}

// --------------------
// PressedEnter
void ChatBox::PressedEnter()
{
	if (!m_typedMsg.size())
		return;

	g_tcpClient.SendChatMsg(m_typedMsg.c_str());
	m_typedMsg.clear();
}