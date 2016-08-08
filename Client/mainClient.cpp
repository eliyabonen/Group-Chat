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

#define PORT 8820
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void handleMessages(SOCKET s);

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

	/*cout << "Enter server ip: ";
	cin >> serverIP;*/

	// Configuration of the socket
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
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

	thread(handleMessages, s).join();

	system("pause");
}

void handleMessages(SOCKET s)
{
	char msg[1024] = { '/0' };

	while (true)
	{
		cout << ">> ";
		cin >> msg;

		if (send(s, msg, 1024, 0) == INVALID_SOCKET)
		{
			cout << "ERROR: INVALID SOCKET!" << endl;
			closesocket(s);

			break;
		}

		if (recv(s, msg, 1024, 0) == INVALID_SOCKET)
		{
			cout << "ERROR: INVALID SOCKET!" << endl;
			closesocket(s);

			break;
		}

		cout << "SOCKET " << s << ": " << msg << endl;
	}
}