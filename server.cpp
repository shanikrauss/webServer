#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "StructsAndDefines.h"
#include "SendMsgs.h"


// הלקוחות מסיימים לא בהכרח לפי הסדר שהם נכנסו
// נחפש את המקום הראשון שפנוי ונוסיף אותו
bool addSocket(SocketState* sockets, SOCKET id, int what, int* socketsCount);

// מקבלים אינדקס ומעיפים אותו מהמערך במקום האינדקס
void removeSocket(SocketState* sockets, int index, int* socketsCount);

void acceptConnection(SocketState* sockets, int index, int* socketsCount);

void copyAllMsgsToStart(SocketState* socket);

void updateSocketState(SocketState* socket, int type);

void receiveMessage(SocketState* sockets, int index, int* socketsCount);

void main()
{
	// Initialize Winsock (Windows Sockets).
	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}
	// שם אותם ככה כי אין לו כוח לשלוח את המערך, צריך להכניס למיין ולהוסיף לפונקציות, כולן צריכות לקבל אותו
// המערך של הלקוחות איפסנו אותו בהתחלה וגם אפס הגדרנו להיות ריקים 
	SocketState sockets[MAX_SOCKETS] = { 0 };
	// אומר כמה במערך תפוסים כאילו כמה לקוחות יש לנו
	int socketsCount = 0;
	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(WEB_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(sockets, listenSocket, LISTEN, &socketsCount);

	// Accept connections and handles them one by one.
	while (true)
	{

		time_t timer;
		time(&timer);
		// checking if 2 minutes have passed  since the last request from socket, if so we close the connection
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].recv != EMPTY && sockets[i].send != EMPTY && timer - sockets[i].timeOfLastReq > 120)
			{
				removeSocket(sockets, i, &socketsCount);
			}
		}

		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
		
		// כאן אנחנו מכינים את "השקים" לפונקציה סלקט 
		fd_set waitRecv; 
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv); // מכניס לשק
		}

		fd_set waitSend;
		FD_ZERO(&waitSend); // מאפס את המשתנה
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		//
		// Wait for interesting event.
		// 
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		//
		int nfd;
		// הדרך שלנו לשאול את המערכת ההפעלה אם משהו יצליח
		// הערך שהיא מחזירה זה כמה תשובות/בקשות יש לנו בשקים
		// כל מי שהתשובה לגביו היא שלילית היא מוציאה מהשק, כלומר היא משאירה בשקים רק את מי שהתשובה היא חיובית הגיעה אליו משהו אפשר לשלוח לו משהו וכו
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		// עבור כל לקוח שהתשובה לגביו הייתה חיובית נעשה את הפעולה הרלוונטית
		// 
		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv)) // מחזירה האם הסוקט נמצא בשק, אם כן הוא דורש טיפול כי הוא נשאר בשק אחרי הפונקציה סלקט
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN: // אם הוא בסטטוס הזה אנחנו מקבלים לקוח חדש
					acceptConnection(sockets, i, &socketsCount);
					break;

				case RECEIVE: // אנחנו צריכים לקבל הודעה, הגיעה הודעה
					receiveMessage(sockets, i, &socketsCount);
					break;
				}
			}
		}


		// כאן נעבור על כל מי שצריך לשלוח לו הודעה, בודק מי נשאר בתוך שק הסנד
		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(sockets, i);
					break;
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

// הלקוחות מסיימים לא בהכרח לפי הסדר שהם נכנסו
// נחפש את המקום הראשון שפנוי ונוסיף אותו
bool addSocket(SocketState* sockets, SOCKET id, int what, int* socketsCount)
{
	//
	// Set the socket to be in non-blocking mode.
	// למקרה קצה
	unsigned long flag = 1;
	if (ioctlsocket(id, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	time_t timer;
	time(&timer);

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what; // status
			sockets[i].send = IDLE; 
			sockets[i].avilable = 0;
			(*socketsCount)++;
			sockets[i].timeOfLastReq = timer;
			return (true);
		}
	}
	return (false); // no space to add new client
}

// מקבלים אינדקס ומעיפים אותו מהמערך במקום האינדקס
void removeSocket(SocketState* sockets, int index, int* socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;

	for (int i = 0; i < MAX_REQUESTS_PER_SOCKET; i++)
	{
		sockets[index].buffer[i][0] = '\0';
		sockets[index].bufferLen[i] = 0;
	}

	(*socketsCount)--;
}

void acceptConnection(SocketState* sockets, int index, int* socketsCount)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	if (addSocket(sockets, msgSocket, RECEIVE, socketsCount) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void copyAllMsgsToStart(SocketState* socket)
{
	for (int i = 1; i < MAX_REQUESTS_PER_SOCKET; i++)
	{
		strcpy(socket->buffer[i - 1], socket->buffer[i]);
		socket->bufferLen[i - 1] = socket->bufferLen[i];
	}

	socket->buffer[MAX_REQUESTS_PER_SOCKET - 1][0] = '\0';
	socket->bufferLen[MAX_REQUESTS_PER_SOCKET - 1] = 0;

	for (int i = 0; i < MAX_REQUESTS_PER_SOCKET; i++)
	{
		if (socket->bufferLen[i] == 0)
		{
			socket->avilable = i;
			break;
		}
	}
}

void updateSocketState(SocketState* socket, int type)
{
	socket->send = SEND;
	socket->sendSubType = type;
	strcpy(socket->lastRecv, socket->buffer[0]);
	copyAllMsgsToStart(socket);
}

void receiveMessage(SocketState* sockets, int index, int* socketsCount)
{
	SOCKET msgSocket = sockets[index].id; // הלקוח שלח לנו הודעה

	int avilable = sockets[index].avilable;

	int bytesRecv = recv(msgSocket, sockets[index].buffer[avilable], sizeof(sockets[index].buffer[avilable]), 0);

	time_t timer;
	time(&timer);
	sockets[index].timeOfLastReq = timer;

	// אנחנו מביאים את ההודעה לבאפר

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(sockets, index, socketsCount);
		return;
	}
	if (bytesRecv == 0) // הלקוח סגר את הקשר ואנחנו נמחק אותו מהמערך שלנו
	{
		closesocket(msgSocket);
		removeSocket(sockets, index, socketsCount);
		return;
	}
	if (sockets[index].avilable == MAX_REQUESTS_PER_SOCKET)
	{
		cout << "\t\tToo many Too many requests!\nThe request was not saved!\n";
	}
	else
	{
		sockets[index].buffer[avilable][bytesRecv] = '\0'; //add the null-terminating to make it a string

		sockets[index].bufferLen[avilable] = bytesRecv;

		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << sockets[index].buffer[avilable] << "\" message.\n";

		sockets[index].avilable++; 

		// כאן מכינים את עצמו לשלב של החזרת התשובה

		if (sockets[index].bufferLen[avilable] > 0)
		{
			// בודקאיזה הודעה שי בבאפר, ומעדכן את הסוקט בהתאם 

			char* buffer = sockets[index].buffer[0];

			if (strncmp(buffer, "GET", 3) == 0)
			{
				updateSocketState(&(sockets[index]), GET);
			}
			else if (strncmp(buffer, "PUT", 3) == 0)
			{
				updateSocketState(&(sockets[index]), PUT);
			}
			else if (strncmp(buffer, "HEAD", 4) == 0)
			{
				updateSocketState(&(sockets[index]), HEAD);
			}
			else if (strncmp(buffer, "POST", 4) == 0)
			{
				updateSocketState(&(sockets[index]), POST);
			}
			else if (strncmp(buffer, "TRACE", 5) == 0)
			{
				updateSocketState(&(sockets[index]), TRACE);
			}
			else if (strncmp(buffer, "DELETE", 6) == 0)
			{
				updateSocketState(&(sockets[index]), DELETE);
			}
			else if (strncmp(buffer, "OPTIONS", 7) == 0)
			{
				updateSocketState(&(sockets[index]), OPTIONS);
			}
			else if (strncmp(buffer, "Exit", 4) == 0)
			{
				closesocket(msgSocket);
				removeSocket(sockets, index, socketsCount);
			}
			else
			{
				updateSocketState(&(sockets[index]), UNKNOWN_REQ);
			}
		}
	}

}
