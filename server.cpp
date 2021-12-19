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


// מוסיפות CLINET חדש
bool addSocket(SOCKET id, int what);
// מורידים CLINET ישן
void removeSocket(int index);

// לקבל לקוח 
void acceptConnection(int index);

//לקבל הודעה
void receiveMessage(int index);

//לשלוח הודעה
//void sendMessage(int index);


// שם אותם ככה כי אין לו כוח לשלוח את המערך, צריך להכניס למיין ולהוסיף לפונקציות, כולן צריכות לקבל אותו
// המערך של הלקוחות איפסנו אותו בהתחלה וגם אפס הגדרנו להיות ריקים 
struct SocketState sockets[MAX_SOCKETS] = { 0 };
// אומר כמה במערך תפוסים כאילו כמה לקוחות יש לנו
int socketsCount = 0;


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
	addSocket(listenSocket, LISTEN);

	// Accept connections and handles them one by one.
	while (true)
	{
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
					acceptConnection(i);
					break;

				case RECEIVE: // אנחנו צריכים לקבל הודעה, הגיעה הודעה
					receiveMessage(i);
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
					sendMessage(i, sockets);
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
bool addSocket(SOCKET id, int what)
{
	//
	// Set the socket to be in non-blocking mode.
	// למקרה קצה
	unsigned long flag = 1;
	if (ioctlsocket(id, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what; // status
			sockets[i].send = IDLE; 
			//sockets[i].len = 0;
			sockets[i].avilable = 0;
			socketsCount++;
			return (true);
		}
	}
	return (false); // no space to add new client
}

// מקבלים אינדקס ומעיפים אותו מהמערך במקום האינדקס
void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;

	for (int i = 0; i < 10; i++)
	{
		sockets[index].buffer[i][0] = '\0';
		sockets[index].bufferLen[i] = 0;
	}

	socketsCount--;
}

void acceptConnection(int index)
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

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void copyAllMsgsToStart(SocketState* socket)
{
	for (int i = 1; i < 10; i++)
	{
		strcpy(socket->buffer[i - 1], socket->buffer[i]);
		socket->bufferLen[i - 1] = socket->bufferLen[i];
	}

	socket->buffer[9][0] = '\0';
	socket->bufferLen[9] = 0;

	for (int i = 0; i < 10; i++)
	{
		if (socket->bufferLen[i] == 0)
		{
			socket->avilable = i;
			break;
		}
	}
}


void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id; // הלקוח שלח לנו הודעה

	int avilable = sockets[index].avilable;

	//int bytesRecv = recv(msgSocket, sockets[index].buffer[avilable], sizeof(sockets[index].buffer) - avilable, 0);
	int bytesRecv = recv(msgSocket, sockets[index].buffer[avilable], sizeof(sockets[index].buffer[avilable]), 0);

	// אנחנו מביאים את ההודעה לבאפר

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0) // הלקוח סגר את הקשר ואנחנו נמחק אותו מהמערך שלנו
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (sockets[index].avilable == 10)
	{
		cout << "\t\tToo many Too many requests!\nThe request was not saved!\n";
	}
	else
	{
		sockets[index].buffer[avilable][bytesRecv] = '\0'; //add the null-terminating to make it a string
		//sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string

		sockets[index].bufferLen[avilable] = bytesRecv;

		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << sockets[index].buffer[avilable] << "\" message.\n";

		//sockets[index].len += bytesRecv;
		sockets[index].avilable++; // = sockets[index].avilable + 1 > 9 ? sockets[index].avilable : sockets[index].avilable + 1;

		// כאן מכינים את עצמו לשלב של החזרת התשובה
		//if (sockets[index].len > 0)
		if (sockets[index].bufferLen[avilable] > 0)
		{
			// בודקאיזה הודעה שי בבאפר, ומעדכן את הסוקט בהתאם 

			// כאן יש באגים באורך ההודעה צריך לעדכן 
			char* buffer = sockets[index].buffer[0];

			if (strncmp(buffer, "GET", 3) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = GET;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				// מעתיק את כל מה שיש אחרי ההודעה של טימסטרינג לתחילת הבאפר לכן 10 כי הגודל של המחרוזת הזאת היא 0 זה ישתנה אצלי
				//memcpy(sockets[index].buffer, &sockets[index].buffer[3], sockets[index].len - 3);
				// אורך ההודעה שטיפלנו
				//sockets[index].len -= 3;
				return;
			}
			else if (strncmp(buffer, "PUT", 3) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = PUT;				
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				//memcpy(sockets[index].buffer, &sockets[index].buffer[3], sockets[index].len - 3);
				//sockets[index].len -= 3;
				return;
			}
			else if (strncmp(buffer, "HEAD", 4) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = HEAD;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);


				//memcpy(sockets[index].buffer, &sockets[index].buffer[4], sockets[index].len - 4);
				//sockets[index].len -= 4;
				return;
			}
			else if (strncmp(buffer, "POST", 4) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = POST;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				//memcpy(sockets[index].buffer, &sockets[index].buffer[4], sockets[index].len - 4);
				//sockets[index].len -= 4;
				return;
			}
			else if (strncmp(buffer, "TRACE", 5) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = TRACE;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				//memcpy(sockets[index].buffer, &sockets[index].buffer[5], sockets[index].len - 5);
				//sockets[index].len -= 5;
				return;
			}
			else if (strncmp(buffer, "DELETE", 6) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = DELETE;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				//memcpy(sockets[index].buffer, &sockets[index].buffer[6], sockets[index].len - 6);
				//sockets[index].len -= 6;
				return;
			}
			else if (strncmp(buffer, "OPTIONS", 7) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = OPTIONS;
				strcpy(sockets[index].lastRecv, sockets[index].buffer[0]);
				copyAllMsgsToStart(&sockets[index]);
				//memcpy(sockets[index].buffer, &sockets[index].buffer[7], sockets[index].len - 7);
				//sockets[index].len -= 7;
				return;
			}
			else if (strncmp(buffer, "Exit", 4) == 0)
			{
				closesocket(msgSocket);
				removeSocket(index);
				return;
			}

		}
	}

}
