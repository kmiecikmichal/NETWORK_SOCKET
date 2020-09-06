#include "Server.h"


// Winsock initialization function 
int WinsockInit(void) {
	WSADATA wsaData;
	int wsaResult;

	// MAKEWORD(2, 2) makes request for WinSock 2.2
	wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (wsaResult != 0){
		printf("ERROR: WSAStartup (winsock) function failed with code: %d\n", wsaResult);
		return 1;
	}

	return 0;
}


int GetSocket(void) {
	// Socket listen object initialize 
	SOCKET serverSocket = INVALID_SOCKET;

	// Create a SOCKET for listening for incoming connection requests.
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serverSocket == INVALID_SOCKET) {
		printf("ERROR: socket (winsock) function failed with code: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	//socket listen is identifier of the socket
	return serverSocket;
}


int ReadTimeout(SOCKET socket, long sec, long usec) {
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;

	// assign the second and microsecond variables
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);

	// Possible return values:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}


int BindSocket(SOCKET socket) {
	int bindResult = SOCKET_ERROR;
	struct sockaddr_in servAddress;

	servAddress.sin_family = AF_INET;
	// converts ip address to binary form 
	inet_pton(AF_INET, (PCSTR)("127.0.0.1"), &servAddress.sin_addr.s_addr);
	// htons makes binary of port number, probably big endian
	servAddress.sin_port = htons(PORT);

	bindResult = bind(socket, (SOCKADDR *)& servAddress, sizeof(servAddress));

	if (bindResult == SOCKET_ERROR) {
		printf("ERROR: bind (winsock) function failed with code: %d\n", WSAGetLastError());
		bindResult = closesocket(socket);
		if (bindResult == SOCKET_ERROR)
			printf("ERROR: closesocket (winsock) function failed with code: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	return 0;
}


int Listening(SOCKET socket) {
	int listenResult = SOCKET_ERROR;

	// somaxcon = max queue value specifiable by listen = max value of clients to stop listening
	listenResult = listen(socket, SOMAXCONN);

	if (listenResult == SOCKET_ERROR) {
		printf("ERROR: listen (winsock) function failed with code: %ld\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return 1;
	}

	return 0;
}


int Accepting(SOCKET socket) {
	SOCKET clientSocket = INVALID_SOCKET;
	DWORD thread;

	while (1) {
		clientSocket = accept(socket, NULL, NULL);

		if (clientSocket == INVALID_SOCKET) {
			printf("ERROR: accept (winsock) function failed with code: %d\n", WSAGetLastError());
			return 1;
		}
		else {
			CreateThread(NULL, 0, ThreadFunc, (LPVOID)clientSocket, 0, &thread);
		}
	}
}


int SendReply(SOCKET socket, char *recvBuffer) {
	SOCKADDR_IN client_info = { 0 };
	int sendResult = SOCKET_ERROR;
	const char *sendBuffer; // reply here
	
	printf("Message received from client: %s\n", recvBuffer);

	// TIME 
	if (strcmp(recvBuffer, "time") == 0) {
		time_t rawTime;
		struct tm *timeInfo;

		time(&rawTime);
		timeInfo = localtime(&rawTime);
		sendBuffer = asctime(timeInfo);
	}
	// CLIENT'S IP ADDRESS
	else if (strcmp(recvBuffer, "myip") == 0) {
		getpeername(&socket, &client_info, sizeof(client_info));
		char *clientIp = inet_ntoa(client_info.sin_addr);
		sendBuffer = clientIp;
	}
	else {
		sendBuffer = "Error: Command is not supported";
	}

	sendResult = send(socket, sendBuffer, (int)strlen(sendBuffer), 0);

	return sendResult; 
}


int Shutdown(SOCKET socket) {
	int shutdownResult = SOCKET_ERROR;

	shutdownResult = shutdown(socket, SD_SEND);

	if (shutdownResult == SOCKET_ERROR) {
		printf("ERROR: shutdown (winsock) function failed with code: %d\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return 1;
	}

	return 0;
}


// Thread function
DWORD WINAPI ThreadFunc(LPVOID clientSocket) {
	char recvBuffer[BUFF_LEN];
	int receiveResult;
	int sendResult = SOCKET_ERROR;

	// Receive until the peer shuts down the connection
	do {
		// fill with Nulls (\0) to avoid getting garbage asciis
		memset(recvBuffer, '\0', BUFF_LEN);

		receiveResult = recv(clientSocket, recvBuffer, BUFF_LEN, 0);
		printf("ReceiveResult %d\n", receiveResult);

		if (receiveResult > 0) {
			sendResult = SendReply(clientSocket, recvBuffer);

			if (sendResult == SOCKET_ERROR) {
				printf("ERROR: send (winsock) function failed with code: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}

			printf("Bytes sent: %d\n", sendResult);
		}
		else if (receiveResult == 0) {
			printf("Closing connection... \n");
			return 0;
		}
		else {
			printf("ERROR: recv (winsock) function failed with code: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

	} while (receiveResult != 0);
	
	return 0;
}


int main() {
	// Win Socket initialization 
	int winsock;
	winsock = WinsockInit();

	// Get Socket
	SOCKET serverSocket;
	serverSocket = GetSocket();

	// Bind socket
	BindSocket(serverSocket);

	// Listen 
	Listening(serverSocket);

	// Accept
	Accepting(serverSocket);

	// Shutdown
	Shutdown(serverSocket);

	system("pause");
}
