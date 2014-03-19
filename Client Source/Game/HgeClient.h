#ifndef _HGECLIENT_H
#define _HGECLIENT_H

#include "stdafx.h"

#define SCREEN_WIDTH g_hgeClient.sW()
#define SCREEN_HEIGHT g_hgeClient.sH()

#define SCREEN_WIDTH_F (float)g_hgeClient.sW()
#define SCREEN_HEIGHT_F (float)g_hgeClient.sH()

#define SCREEN_C_X (SCREEN_WIDTH_F / 2.0f)
#define SCREEN_C_Y (SCREEN_HEIGHT_F / 2.0f)

#define TEX_HALF_W(a) (a->GetWidth() / 2.0f)
#define TEX_HALF_H(a) (a->GetHeight() / 2.0f)

#define _hge g_hgeClient.m_hge
#define _input g_hgeClient.m_input

#define CENTER_SPRITE_HOTSPOT(a) a->SetHotSpot(a->GetWidth() / 2.0f, a->GetHeight() / 2.0f)

struct Texture
{
	Texture() : sprite(0) {}

	hgeSprite* sprite;
	HTEXTURE texture;
	std::string directory;
};

struct Font
{
	Font() : font(0) {}

	hgeFont* font;
	std::string directory;
};

struct Sound
{
	Sound() : sound(0) {}

	HSTREAM sound;
	std::string directory;
};

class InputBox;

class HgeClient
{
	public:
		HgeClient(int width, int height);

		void Begin();
		void ToggleSound(std::string sound, bool bApply, int VOLUME = 100);

		void changeInputBox(InputBox* pBox) { m_inputBox = pBox; }

		static bool FrameFunc();
		static bool RenderFunc();
		
		int sW() const { return m_iScreenWidth; }
		int sH() const { return m_iScreenHeight; }
				
		hgeSprite* sprite(std::string dir) const
		{
			for (unsigned int i = 0; i < m_vTextures.size(); ++i)
				if (m_vTextures[i].directory.find(dir) != std::string::npos)
					return m_vTextures[i].sprite;
			return 0;
		}

		hgeFont* font(std::string dir) const
		{
			for (unsigned int i = 0; i < m_vFonts.size(); ++i)
				if (m_vFonts[i].directory.find(dir) != std::string::npos)
					return m_vFonts[i].font;
			return 0;
		}

		InputBox* curInputBox() const { return m_inputBox; }

		HGE* m_hge;
		hgeInputEvent m_input;

	protected:
		void LoadAllContent();
		void UnloadAllContent();		
		void LoadTexture(Texture* texture, std::string dir);
		
		float getTWidth(HTEXTURE tex) const { return (float)m_hge->Texture_GetWidth(tex); }
		float getTHeight(HTEXTURE tex) const { return (float)m_hge->Texture_GetHeight(tex); }

		void GetFileList(const char *param, std::vector<std::string>& vAllFiles);

		bool m_bReload;

		int m_iScreenWidth;
		int m_iScreenHeight;

		InputBox* m_inputBox;

		std::vector<Font> m_vFonts;
		std::vector<Texture> m_vTextures;
		std::vector<Sound> m_vSounds;
};

#endif