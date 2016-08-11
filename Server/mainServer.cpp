#include <iostream>
#include <WinSock2.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <string>
#include "RecievedMessage.h"

#define PORT 8820
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET _socket;
vector<SOCKET> _connectedClients;
mutex _mtxRecievedMessages;
queue<RecievedMessage*> _queRcvMessages;
mutex _mtxRecievedMessagesHandler;
condition_variable cv;

void bindAndListen();
void acceptClient();
void clientHandler(SOCKET s);
void handleRecievedMessages();
void addRecievedMessage(RecievedMessage* rcvMsg);
RecievedMessage* buildRecievedMessage(char* recvMsg, SOCKET s);

int main()
{
	bindAndListen();

	// creating the thread of the message handling, eliya can create it here because it's only triggered when i wake up the condition variable
	thread(handleRecievedMessages).detach();
	thread(acceptClient).join();
}

void bindAndListen()
{
	WSADATA wsa;
	struct sockaddr_in server;

	cout << "Initialising Winsock...\n";

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Failed. Error Code : " << WSAGetLastError();
		WSACleanup();
		return;
	}

	cout << "Initialized.\n";

	// Create a socket
	if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		cout << "Could not create socket : " << WSAGetLastError();

		closesocket(_socket);
		WSACleanup();

		return;
	}

	cout << "Socket created.\n";

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	
	if (::bind(_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		cout << "Bind failed with error code : " << WSAGetLastError();
		closesocket(_socket);
		WSACleanup();
		return;
	}

	cout << "Bind done\n";

	//Listen to incoming connection
	listen(_socket, 1);

}

void acceptClient()
{
	// accepting new clients
	while (true)
	{
		// creating their individual socket
		struct sockaddr_in client;
		int c = sizeof(struct sockaddr_in);
		SOCKET new_socket = ::accept(_socket, (struct sockaddr *)&client, &c);

		if (new_socket == INVALID_SOCKET)
		{
			cout << "accept failed with error code : " << WSAGetLastError();
			closesocket(_socket);
			WSACleanup();
			return;
		}

		_connectedClients.push_back(new_socket);
		thread(clientHandler, new_socket).detach();

		cout << "Connection accepted ,socket: " << new_socket << endl;
	}
}

void clientHandler(SOCKET s)
{
	try {

		// getting messages from the client
		while (true)
		{
			char recvMsg[1024] = { '/0' };

			int res = recv(s, recvMsg, 1024, 0);

			if (res == INVALID_SOCKET)
			{
				cout << "in the recv error if" << endl;

				string errMsg = "Error while receiving from socket: ";
				errMsg.append(to_string(s));

				cout << errMsg << endl;
				cout << "last error: " << WSAGetLastError() << endl;
				closesocket(s);
				return;
			}

			// if sent "end" ending the conversation
			if (recvMsg[0] == 'e' && recvMsg[1] == 'n' && recvMsg[2] == 'd')
			{
				cout << "Client " << s << " left the chat" << endl;

				// removing from the users vector
				for (int i = 0; i < _connectedClients.size(); i++)
				{
					if (s == _connectedClients.at(i))
					{
						_connectedClients.erase(_connectedClients.begin() + i);
						break;
					}
				}

				// closing the socket
				closesocket(s);
				this_thread::sleep_for(chrono::milliseconds(200));

				return;
			}

			// adding to recieved messages
			addRecievedMessage(buildRecievedMessage(recvMsg, s));
			this_thread::sleep_for(chrono::milliseconds(200));
		}
	}
	catch (...)
	{
		closesocket(s);
	}
}

void handleRecievedMessages()
{
	string str;

	while (true)
	{
		// waiting until there is a new message
		unique_lock<mutex> lck(_mtxRecievedMessagesHandler);
		cv.wait(lck);

		// if the queue is not empty, then take care of all the messages
		while (_queRcvMessages.empty() != true)
		{
			// getting the first message
			RecievedMessage* message = _queRcvMessages.front();
			_queRcvMessages.pop();

			str.append("Client ");
			str.append(to_string(message->getSock()));
			str.append(": ");
			str.append(message->getMsg());

			cout << str << endl;

			// sending the message to all except the one who sent the message
			for (int i = 0; i < _connectedClients.size(); i++)
			{
				// if it is not the one who sent the message
				if (_connectedClients.at(i) != message->getSock())
				{
					// sending the message to the client in the current iteration
					if (send(_connectedClients.at(i), str.c_str(), 1024, 0) == INVALID_SOCKET)
					{
						cout << "ERROR: INVALID SOCKET!" << endl;
						closesocket(message->getSock());

						break;
					}
				}
			}

			str.clear();
		}

		lck.unlock();
	}
}

RecievedMessage* buildRecievedMessage(char* recvMsg, SOCKET s)
{
	return (new RecievedMessage(s, recvMsg));
}

void addRecievedMessage(RecievedMessage* rcvMsg)
{
	_mtxRecievedMessages.lock();
	_queRcvMessages.push(rcvMsg);
	_mtxRecievedMessages.unlock();

	// now there is a new message, i'm waking up the thread to take care of that message
	cv.notify_all();
}