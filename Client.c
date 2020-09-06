#include "Client.h"


// Winsock initialization function 
int WinsockInit(void){
	WSADATA wsaData;
	int result;

	// MAKEWORD(2, 2) makes request for WinSock 2.2
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0){
		printf("ERROR: WSAStartup (winsock) function failed with code: %d\n", result);
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


int ConnectSocket(SOCKET socket) {
	int connectResult;
	struct sockaddr_in servAddress;
	const char serverAddressFromFile[BUFF_LEN]; //here goes server address from file
	FILE *fileptr;

	// Get server address from file
	fileptr = fopen("server_address.txt", "r");

	if (!fileptr) {
		printf("ERROR: Cannot read the config file");
		exit(1);
	}

	fscanf(fileptr, "%[^\n]", serverAddressFromFile);
	fclose(fileptr);


	servAddress.sin_family = AF_INET;
	// converts ip address to binary form 
	inet_pton(AF_INET, (PCSTR)(serverAddressFromFile), &servAddress.sin_addr.s_addr);
	// htons makes binary of port number, probably big endian
	servAddress.sin_port = htons(PORT);


	connectResult = connect(socket, (struct sockaddr *) &servAddress, sizeof(servAddress));

	if (connectResult == SOCKET_ERROR) {
		printf("ERROR: connect (winsock) function failed with code: %d\n", WSAGetLastError());
		connectResult = closesocket(socket);
		socket = INVALID_SOCKET;

		if (connectResult == SOCKET_ERROR)
			printf("ERROR: closesocket (winsock) function failed with code: %d\n", WSAGetLastError());

		WSACleanup();
		return 1;
	}

	if (socket == INVALID_SOCKET) {
		printf("ERROR: Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	return 0;
}


int Messaging(SOCKET socket, const char *sendBuffer) {
	char receiveBuffer[BUFF_LEN];
	int receiveResult = 0;
	int sendResult = SOCKET_ERROR;

	sendResult = send(socket, sendBuffer, (int)strlen(sendBuffer), 0);

	if (sendResult == SOCKET_ERROR) {
		printf("ERROR: send (winsock) function failed with code: %d\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return 1;
	}
	
	do {
		memset(receiveBuffer, '\0', BUFF_LEN);

		int timeout = ReadTimeout(socket, 1, 0);

		if (timeout == -1) {
			printf("ERROR: timeout (winsock) function failed with code: %d\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			return 1;
		}
		else if (timeout == 0) {
			//printf("TIMEOUT");
			break;
		}
		else {
			receiveResult = recv(socket, receiveBuffer, BUFF_LEN, 0);
		}

		if (receiveResult > 0) {
			printf("%s", receiveBuffer);
		}
		else if (receiveResult == 0) {
			printf("Connection closed\n");
		}
		else {
			printf("ERROR: recv (winsock) function failed with code: %d\n", WSAGetLastError());
		}
	} while (receiveResult > 0);
	

	return 0;
}

int ClientMenu(SOCKET socket) {
	const char *sendBuffer;
	int menuChoice = 0;

	// MENU
	do {
		printf("\n--------Menu---------\n\n");
		printf("1. Time .............\n");
		printf("2. My IP ............\n");
		printf("3. Exit .............\n");
		scanf_s("%d", &menuChoice);

		switch (menuChoice) {
		case 1: sendBuffer = "time";
			Messaging(socket, sendBuffer);
			break;
		case 2: sendBuffer = "myip";
			Messaging(socket, sendBuffer);
			break;
		case 3: printf("Client is shutting down...\n");
			return -1;
			break;
		default: printf("Invalid command\n");
			break;
		}
	} while (menuChoice != 3);

	return 0;
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


int main() {
	// Win Socket initialization 
	WinsockInit();

	// Get Socket
	SOCKET clientSocket;
	clientSocket = GetSocket();

	// Connect to server
	ConnectSocket(clientSocket);

	// Talk with server (choose option from menu)
	ClientMenu(clientSocket);

	// Shutdown
	Shutdown(clientSocket);

	system("pause");
}
