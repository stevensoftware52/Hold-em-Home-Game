// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <direct.h>
#include <strsafe.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <assert.h>
#include <process.h>
#include <algorithm>

using namespace std;

typedef unsigned char uint8;

/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**/

#include "C:\Software Development\JokerStars\JokerStars\JokerStars\include\hge.h"
#include "C:\Software Development\JokerStars\JokerStars\JokerStars\include\hgecolor.h"
#include "C:\Software Development\JokerStars\JokerStars\JokerStars\include\hgesprite.h"
#include "C:\Software Development\JokerStars\JokerStars\JokerStars\include\hgedistort.h"
#include "C:\Software Development\JokerStars\JokerStars\JokerStars\include\hgefont.h"

/*
** Copyright (C) 2014 Steven Rogers
**/

#include "Util.h"

#include "Game\\ClientCard.h"
#include "Game\\CardLibrary.h"
#include "Game\\HgeClient.h"
#include "Game\\InputBox.h"
#include "Game\\TableRender.h"
#include "Game\\ChatBox.h"
#include "Game\\Player.h"

#include "Networking\\TcpClient.h"
#include "Networking\\LoginMenu.h"

#include "Game\\Button.h"
#include "Game\\GameInfo.h"
#include "Game\\Slider.h"

extern HgeClient g_hgeClient;
extern TableRender g_tableRender;
extern ChatBox g_chatBox;
extern CardLibrary g_cardLibrary;
extern TcpClient g_tcpClient;
extern LoginMenu g_loginMenu;
extern GameInfo g_gameInfo;