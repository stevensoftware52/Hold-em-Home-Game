// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

TcpMgr g_tcpMgr;
GameFloor g_gameFloor;

HANDLE hEvent;
bool bLoop = true;

BOOL WINAPI CtrlHandler(DWORD dwType) // On console close
{
    printf("Exiting program...\n");

	bLoop = false;

    Sleep(1000);    
    SetEvent(hEvent);

    return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

	DWORD lastTime = clock();

    while (bLoop)
    {
        Sleep(100);

		DWORD clockBefore = clock();
        
        g_tcpMgr.Update();
        g_gameFloor.Update(clock() - lastTime);

		DWORD clockMinus = clock() - clockBefore;

        lastTime = clock() - clockMinus;
    }

	return 0;
}