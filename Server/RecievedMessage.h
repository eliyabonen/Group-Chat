#pragma once
#ifndef RECIEVEDMESSAGE_H
#define RECIEVEDMESSAGE_H
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

using namespace std;
class RecievedMessage
{
public:
	RecievedMessage(SOCKET s, char* msg);
	SOCKET getSock();
	char* getMsg();
private:
	SOCKET _sock;
	char* _message;
};
#endif