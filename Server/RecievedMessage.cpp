#include "RecievedMessage.h"

RecievedMessage::RecievedMessage(SOCKET s, char* msg) : _sock(s), _message(msg)
{}

SOCKET RecievedMessage::getSock()
{
	return _sock;
}

char* RecievedMessage::getMsg()
{
	return _message;
}
