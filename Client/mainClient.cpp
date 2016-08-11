#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <string>
#include <conio.h>

#define PORT 8820
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void sendMessages(SOCKET s);
void recieveMessages(SOCKET s);

bool breakAll = false;

void main()
{
	WSADATA info;
	string serverIP;
	struct sockaddr_in clientService;
	SOCKET s;

	// Configuration of the socket type

	if (WSAStartup(MAKEWORD(2, 2), &info) != 0)
	{
		cout << "Failed. Error Code : " << WSAGetLastError();
		return;
	}

	// Creating the socket

	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		cout << "Could not create socket : " << WSAGetLastError();

		closesocket(s);
		WSACleanup();

		return;
	}

	cout << "Socket function succeeded\n" << endl;

	cout << "Enter server ip: ";
	cin >> serverIP;

	// Configuration of the socket
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientService.sin_addr.s_addr = inet_addr(serverIP.c_str());
	clientService.sin_port = htons(PORT);

	// Connection request
	if (connect(s, (struct sockaddr*) &clientService, sizeof(clientService)) == SOCKET_ERROR)
	{
		cout << "Failed at connect function, ERROR: %d\n" << WSAGetLastError() << endl;

		// checking if the closing of the socket succeeded
		if (closesocket(s) == SOCKET_ERROR)
			cout << "Failed at closesocket function, ERROR: %d\n" << WSAGetLastError() << endl;
		WSACleanup();

		return;
	}

	cout << "Acquired connection with " << serverIP << endl;

	thread(sendMessages, s).detach();
	thread(recieveMessages, s).detach();

	while (!breakAll);
	system("pause");
}

void sendMessages(SOCKET s)
{
	char msg[1024] = { '/0' };
	int i = 0;

	while (true)
	{
		char c = _getch();

		// if clicked escape, then delete the last character
		if (c == 27)
			msg[strlen(msg) - 1] = '\0';

		// if the user pressed enter
		if (c == 13)
		{
			// print it
			msg[i] = '\0';
			cout << "You: " << msg << endl;
			i = 0;

			// send it
			if (send(s, msg, 1024, 0) == INVALID_SOCKET)
			{
				cout << "ERROR: INVALID SOCKET!" << endl;
				closesocket(s);

				break;
			}

			if (msg[0] == 'e' && msg[1] == 'n' && msg[2] == 'd')
			{
				this_thread::sleep_for(chrono::milliseconds(200));

				closesocket(s);
				exit(0);
			}
		}
		else
			msg[i++] = c;
	}
}

void recieveMessages(SOCKET s)
{
	char msg[1024] = { '/0' };

	while (true)
	{
		if (recv(s, msg, 1024, 0) == INVALID_SOCKET)
		{
			cout << "ERROR: INVALID SOCKET!" << endl;
			closesocket(s);

			break;
		}

		cout << msg << endl;
	}
}