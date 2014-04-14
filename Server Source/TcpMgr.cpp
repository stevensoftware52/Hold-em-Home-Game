#include "stdafx.h"

// --------------
// Constructor
TcpMgr::TcpMgr()
{
    // Startup WSA
    DWORD result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);

    if (result != NO_ERROR) 
    {
        printf("WSAStartup failed with error: %d\n", result);
        system("pause");
    }

    // Create our mutex
    m_mutex = CreateMutex(NULL, FALSE, NULL); 
    
    // Launch the listen thread
    _beginthread(StaticListenThread, 0, 0);	
}

// -----------------
// Destructor
TcpMgr::~TcpMgr()
{
    // Cleanup m_mutex
    CloseHandle(m_mutex);

    // Cleanup any connections
    for (unsigned int i = 0; i < m_vTcpConnections.size(); ++i)
        delete m_vTcpConnections[i];

    m_vTcpConnections.clear();

	WSACleanup();
}

// -----------------
// Update
void TcpMgr::Update()
{    
    WaitForSingleObject(m_mutex, INFINITE);

    {
        auto itr = m_vTcpConnections.begin();

        while (itr != m_vTcpConnections.end())
        {
            // If safe to delete
            if ((*itr)->safeToDelete())
            {
                // If hasn't been removed from g_gameFloor
                if ((*itr)->m_socket != INVALID_SOCKET)
                {
					// Note that RemoveSession will call closesocket
                    g_gameFloor.RemoveSession((*itr)->getAccountId());
                }

                // Delete & erase
                delete (*itr);
                itr = m_vTcpConnections.erase(itr);
            }
            else
            {
                (*itr)->ProcessPendingMsgs();
                ++itr;
            }
        }
    }
    
    ReleaseMutex(m_mutex);
}

// -----------------------
// NewTcpConnection
void TcpMgr::NewTcpConnection(SOCKET sock, unsigned int account_id, float fBankroll, uint8 uiAvatar, std::string username)
{    
    WaitForSingleObject(m_mutex, INFINITE);

    {
        m_vTcpConnections.push_back(new TcpConnection(account_id, sock, fBankroll, uiAvatar, username));
    }

    ReleaseMutex(m_mutex);
}

// -----------------
// StaticListenThread
//      Listens for incoming connections on port TCPMGR_LISTEN_PORT
void TcpMgr::StaticListenThread(LPVOID lpParam)
{
    struct sockaddr_in local, client;

    // Create our listening socket
    //

    SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (lsock == SOCKET_ERROR)
    {
        printf("ListenThread socket() failed: %d\n", WSAGetLastError());
        return;
    }

    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_family = AF_INET;
    local.sin_port = htons(TCPMGR_LISTEN_PORT);

    if (bind(lsock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("ListenThread bind() failed: %d\n", WSAGetLastError());
        return;
    }

    listen(lsock, 16);

    printf("\nTcpMgr listening on port %d\n\n", TCPMGR_LISTEN_PORT);
		   
	// Listen
	//		TODO: Can we check that lsock is still valid now and then?
	//

    while (true)
    {
        int addrsize = sizeof(client);
        SOCKET csock = accept(lsock, (struct sockaddr *)&client, &addrsize);
		
        if (csock != INVALID_SOCKET)
        {       
            int i = 1;
            setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (char*) &i, sizeof(i));        
            _beginthread(StaticAuthThread, 0, (LPVOID)csock);
        }
        else
            closesocket(csock);
    }
	
    closesocket(lsock);
}

// -----------------
// StaticAuthThread
//      Accepts 1 byte (len of username) then accepts username, then accepts 1 byte (len of password) then accepts password
//      Next, it trys to verify those credentials. If do, we ship it
void TcpMgr::StaticAuthThread(LPVOID lpParam)
{
    SOCKET csock = (SOCKET)lpParam;
    char* buffer = new char[MAX_BUFFER_SIZE];
    bool success = false;
    char *clBuffer = 0;
    int clSize = 0;

    #define     USERNAME_MAX_LEN    25
    #define     PASSWORD_MAX_LEN    25

    // Flag socket to timeout after X seconds
    struct timeval tv;
    tv.tv_sec = 30000;
    setsockopt(csock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

    // Recv 1 byte for len of username
    if (Util::recvBytes(csock, buffer, 1) == 1)
    {
        char username_len = *(char*)buffer;

        // Recv username
        if (username_len <= USERNAME_MAX_LEN && Util::recvBytes(csock, buffer, username_len) == username_len)
        {
            std::string username;

            for (int i = 0; i < username_len; ++i)
                username.push_back(buffer[i]);
            
            // Recv 1 byte for len of password
            if (Util::recvBytes(csock, buffer, 1) == 1)
            {
                char password_len = *(char*)buffer;

                // Recv password
                if (password_len <= PASSWORD_MAX_LEN && Util::recvBytes(csock, buffer, password_len) == password_len)
                {
                    std::string password;

                    for (int i = 0; i < password_len; ++i)
                        password.push_back(buffer[i]);
					
					uint8 uiAvatar = 0;
					float fBankroll = 0.0f;

					// Verify creditials
					if (unsigned int account_id = AuthenticateCredentials(username, password, fBankroll, uiAvatar))
						g_tcpMgr.NewTcpConnection(csock, account_id, fBankroll, uiAvatar, username);
					else
						closesocket(csock);
                }
            }
        }
    }

    delete [] buffer;
    delete [] clBuffer;
}

// --------------------------
// AuthenticateCredentials
//		Not using a database or any meaningful secruity
unsigned int TcpMgr::AuthenticateCredentials(std::string username, std::string password, float& fBankroll, uint8& uiAvatar)
{
	// Max len
	if (username.size() > 24 || password.size() > 24)
		return false;

	// No unexpected characters please
	//

	for (uint8 i = 0; i < username.size(); ++i)
		if (!isalpha(username[i]) && !isdigit(username[i]))
			return false;

	for (uint8 i = 0; i < password.size(); ++i)
		if (!isalpha(password[i]) && !isdigit(password[i]))
			return false;
		
	// See if they have an account
	// We determine this by whether or not Username.txt exists
	
	unsigned int uiResult = 0;

	char filename[256];
	sprintf_s(filename, "Users\\%s.txt", username.c_str());
		
	ifstream myfile(filename);

	if (myfile.is_open())
	{
		std::string line;
		std::vector<string> userInformation;

		while (getline (myfile, line))
		{
			userInformation.push_back(line);

			if (userInformation.size() >= NUM_USERFILE_CONTENTS)
				break;
		}

		// Check data
		if (userInformation.size() != NUM_USERFILE_CONTENTS)
			printf("Unexpected end of user contents\n");
		else if (userInformation[0] != password)
			printf("Bad password for %s\n", username.c_str());

		// Data is good, go for the gold
		else
		{
			uiResult = atoi(userInformation[1].c_str());
			fBankroll = (float)atof(userInformation[2].c_str());
			uiAvatar = atoi(userInformation[3].c_str());
		}

		myfile.close();
	}
	else
	{
		// They don't have an account, create one
					
		fstream filestr;
		filestr.open(filename, fstream::in | fstream::out | fstream::app);

		if (filestr.is_open())
		{
			uiResult = PumpNewAccountId();
			filestr << password << '\n' << uiResult << '\n' << "0" << '\n' << "0";
			filestr.close();
		}
	}

	return uiResult;
}

// ------------------
// PumpNewAccountId
unsigned int TcpMgr::PumpNewAccountId()
{	
	unsigned int accountId = 0;
	
	// Load what the max known account id is

	#define ACCOUNTID_FILENAME "accountId.txt"

	ifstream myfile(ACCOUNTID_FILENAME);

	if (myfile.is_open())
	{
		std::string line;
		getline(myfile, line);
		accountId = atoi(line.c_str());
		myfile.close();
	}
	else
	{
		printf("CRASH: Missing %s\n", ACCOUNTID_FILENAME);
		assert(0);
	}

	// Icrement it and write back

	++accountId;

	ofstream newfile(ACCOUNTID_FILENAME);

	if (newfile.is_open())
	{
		newfile << accountId;
		newfile.close();
	}
	else
	{
		printf("CRASH: Failed to overwrite %s\n", ACCOUNTID_FILENAME);
		assert(0);
	}

	return accountId;
}