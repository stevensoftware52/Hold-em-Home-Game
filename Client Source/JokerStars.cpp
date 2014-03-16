// JokerStars.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

HgeClient g_hgeClient(1080, 800);
TableRender g_tableRender;
ChatBox g_chatBox;
CardLibrary g_cardLibrary;
TcpClient g_tcpClient;
LoginMenu g_loginMenu;
GameInfo g_gameInfo;	

int _tmain(int argc, _TCHAR* argv[])
{		
	FreeConsole();

	string line;
	ifstream myfile ("Content\\Console Logo.txt");

	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			cout << line << '\n';
		}

		myfile.close();
	}
	else
		printf("Missing file Console Logo.txt\n");

	//g_tcpClient.AttemptConnection("g", "g");
	g_hgeClient.Begin();

	return 0;
}

