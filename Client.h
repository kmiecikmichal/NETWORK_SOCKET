#ifndef _CLIENT_
#define _CLIENT_

#pragma warning(disable : 4996)

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#define PORT 2300
#define BUFF_LEN 512

int WinsockInit(void);
int GetSocket(void);
int ReadTimeout(SOCKET socket, long sec, long usec);
int ConnectSocket(SOCKET socket);
int Messaging(SOCKET socket, const char *sendBuffer);
int ClientMenu(SOCKET socket);
int Shutdown(SOCKET socket);

#endif 