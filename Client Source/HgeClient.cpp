#include "stdafx.h"

// -------------------
// Constructor
HgeClient::HgeClient(int width, int height) :
	m_iScreenWidth(width),
	m_iScreenHeight(height)
{
	m_bReload = false;
	m_inputBox = 0;
}

// ------------------
// Begin
void HgeClient::Begin()
{
	do
	{
		m_bReload = false;

		printf("Launching DirectX.\n");

		if (m_hge = hgeCreate(HGE_VERSION))
		{
			m_hge->System_SetState(HGE_DONTSUSPEND, true);
			m_hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
			m_hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
			m_hge->System_SetState(HGE_TITLE, "MF Poker 'n Sons");
			m_hge->System_SetState(HGE_USESOUND, true);
			m_hge->System_SetState(HGE_WINDOWED, true);
			m_hge->System_SetState(HGE_SCREENWIDTH, m_iScreenWidth);
			m_hge->System_SetState(HGE_SCREENHEIGHT, m_iScreenHeight);
			m_hge->System_SetState(HGE_SCREENBPP, 32);
			m_hge->System_SetState(HGE_FPS, 144);
			m_hge->System_SetState(HGE_SHOWSPLASH, false);
			m_hge->System_SetState(HGE_ICON, MAKEINTRESOURCE(101));
			
			if (m_hge->System_Initiate())
			{
				LoadAllContent();
				ShowWindow(GetConsoleWindow(), SW_HIDE);
				m_hge->System_Start();
			}
		
			UnloadAllContent();

			m_hge->System_Shutdown();
			m_hge->Release();
		}

		Sleep(1000);
	}
	while (false); // TODO Implement reopning after changing resolution
}

// -------------------
// FrameFunc
bool HgeClient::FrameFunc()
{
	_hge->Input_GetEvent(&_input);

	// Current input box
	if (InputBox* pInputBox = g_hgeClient.curInputBox())
		pInputBox->Input();

	// Gameinfo
	g_gameInfo.Update();

	if (g_tcpClient.m_bConnected)
	{
		// Table
		g_tableRender.InterfaceInput();
	
		// Chatbox
		g_chatBox.Input();

		// TcpClient logic on main thread
		g_tcpClient.Update(_hge->Timer_GetDelta());

		// Debug Commands
		;
	}
	else
	{
		g_loginMenu.Input();
	}

	return false;
}

// -------------------
// RenderFunc
bool HgeClient::RenderFunc()
{
	_hge->Gfx_BeginScene();

	{
		// Table
		g_tableRender.Render();

		// Chatbox
		g_chatBox.Render();

		// Current input box
		if (InputBox* pInputBox = g_hgeClient.curInputBox())
			pInputBox->Render();

		// Login menu
		if (!g_tcpClient.m_bConnected)
			g_loginMenu.Render();

		// Cursor
		if (hgeSprite* pSprite = g_hgeClient.sprite("cursor.png"))
			pSprite->Render(_input.x, _input.y);

		// Gameinfo text
		g_gameInfo.RenderText();

		// Gameinfo animtions
		g_gameInfo.PumpAnimations();
	}	

	_hge->Gfx_EndScene();

	return false;
}

// -------------------
// LoadAllContent
void HgeClient::LoadAllContent()
{
	// Textures

	printf("Loading all textures.\n");

	std::vector<std::string> vTextureDirectories;
	GetFileList("Content//Textures", vTextureDirectories);

	for (unsigned int i = 0; i < vTextureDirectories.size(); ++i)
	{
		Texture texture;
		LoadTexture(&texture, vTextureDirectories[i].c_str());
		m_vTextures.push_back(texture);
	}

	// Fonts

	printf("Loading all fonts.\n");

	std::vector<std::string> vFontDirectories;
	GetFileList("Content//Fonts", vFontDirectories);

	for (unsigned int i = 0; i < vFontDirectories.size(); ++i)
	{
		if (vFontDirectories[i].find(".fnt") == std::string::npos)
			continue;

		Font font;

		font.font = new hgeFont(vFontDirectories[i].c_str());
		font.directory = vFontDirectories[i];

		m_vFonts.push_back(font);
	}

	// Sounds

	printf("Loading all sounds.\n");

	std::vector<std::string> vSoundDirectories;
	GetFileList("Content//Sounds", vSoundDirectories);

	for (unsigned int i = 0; i < vSoundDirectories.size(); ++i)
	{
		Sound sound;
		sound.directory = vSoundDirectories[i];
		m_vSounds.push_back(sound);
	}	
}

// -------------------
// UnloadAllContent
void HgeClient::UnloadAllContent()
{
	// Textures

	for (unsigned int i = 0; i < m_vTextures.size(); ++i)
	{
		delete m_vTextures[i].sprite;
		m_hge->Texture_Free(m_vTextures[i].texture);
	}

	m_vTextures.clear();

	// Fonts

	for (unsigned int i = 0; i < m_vFonts.size(); ++i)
		delete m_vFonts[i].font;

	m_vFonts.clear();

	// Sounds

	for (unsigned int i = 0; i < m_vSounds.size(); ++i)
		if (m_vSounds[i].sound)
			_hge->Stream_Free(m_vSounds[i].sound);

	m_vSounds.clear();
}

// -------------------
// LoadTexture
void HgeClient::LoadTexture(Texture* texture, std::string dir)
{
	if (texture->texture = m_hge->Texture_Load(dir.c_str()))
	{
		texture->sprite = new hgeSprite(texture->texture, 0, 0, getTWidth(texture->texture), getTHeight(texture->texture));
		texture->directory = dir;
	}
	else
		printf("ERROR: Failed to load texture %s\n", dir.c_str());
}

// ----------------
// ToggleSound
void HgeClient::ToggleSound(std::string dir, bool bApply, int VOLUME)
{
	for (unsigned int i = 0; i < m_vSounds.size(); ++i)
	{
		if (m_vSounds[i].directory.find(dir) != std::string::npos)
		{
			if (m_vSounds[i].sound)
			{
				_hge->Stream_Free(m_vSounds[i].sound);
				m_vSounds[i].sound = 0;
			}

			if (bApply)
			{
				if (m_vSounds[i].sound = _hge->Stream_Load(m_vSounds[i].directory.c_str()))
					_hge->Stream_Play(m_vSounds[i].sound, false, VOLUME);
			}

			return;
		}
	}
}

// -------------------
// GetFileList
void HgeClient::GetFileList(const char *param, std::vector<std::string> &vAllFiles)
{
	std::vector<WIN32_FIND_DATA> visibleFiles;

	// Fetch our current directory
	char currentPath[_MAX_PATH];
	_getcwd(currentPath, _MAX_PATH);

	char addr[_MAX_PATH];
	sprintf_s(addr, "%s//%s", currentPath, param);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError;
	LPSTR DirSpec;
	size_t length_of_arg;
 
	DirSpec = (LPSTR) malloc (MAX_PATH); 
 
	// Check that the input is not larger than allowed.
	StringCbLength(addr, MAX_PATH, &length_of_arg);
	if (length_of_arg > (MAX_PATH - 2))
	{
		printf("Input directory is too large. Error is %u.\nFile = '%s'\n\n", GetLastError(), param);
		return;
	}
 
	// Prepare string for use with FindFile functions.  First, 
	// copy the string to a buffer, then append '\*' to the 
	// directory name.
	StringCbCopyN (DirSpec, MAX_PATH, addr, length_of_arg+1);
	StringCbCatN (DirSpec, MAX_PATH, "\\*", 3);
 
	// Find the first file in the directory.
	hFind = FindFirstFile(DirSpec, &FindFileData);
	
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		printf("Invalid file handle. Error is %u.\nFile = '%s'\n\n", GetLastError(), param);
		return;
	} 
	else
	{
		if (FindFileData.cFileName[0] != '.')
			visibleFiles.push_back(FindFileData);
        
		// List all the other files in the directory.
		while (FindNextFile(hFind, &FindFileData) != 0) 
		{
			if (FindFileData.cFileName[0] != '.')
				visibleFiles.push_back(FindFileData);
		}
		
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			printf("FindNextFile error. Error is %u.\nFile = '%s'\n\n", GetLastError(), param);
		}
	}
 
	free(DirSpec);

	for (unsigned int i = 0; i < visibleFiles.size(); ++i)
	{
		std::string dir = param;

		dir.append("//");
		dir.append(visibleFiles[i].cFileName);

		// If folder, recursive this function
		if (visibleFiles[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			GetFileList(dir.c_str(), vAllFiles);
		}
		else
		{
			vAllFiles.push_back(dir);
		}
	}
}