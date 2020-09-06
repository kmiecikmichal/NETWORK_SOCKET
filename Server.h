#ifndef _SERVER_
#define _SERVER_

#pragma warning(disable : 4996)

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define PORT 2300
#define BUFF_LEN 512

int WinsockInit(void);
int GetSocket(void);
int ReadTimeout(SOCKET socket, long sec, long usec);
int BindSocket(SOCKET socket);
int Listening(SOCKET socket);
int Accepting(SOCKET socket);
int SendReply(SOCKET socket, char *recvBuffer);
int Shutdown(SOCKET socket);

DWORD WINAPI ThreadFunc(LPVOID clientSocket);

#endif 