// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <vector>
#include <process.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

typedef unsigned char uint8;
typedef int PENNY;

using namespace std; 

// additional headers
//

#include "Util.h"
#include "CardDeck.h"
#include "TcpMgr.h"
#include "TcpConnection.h"
#include "GameFloor.h"
#include "Session.h"
#include "Game.h"

// global classes

extern TcpMgr g_tcpMgr;
extern GameFloor g_gameFloor;