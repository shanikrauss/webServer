﻿#include "SendMsgs.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <winsock2.h>

#include "StructsAndDefines.h"

void updateFileContent(char* buffer, int bufLen, FILE* file)
{
	int fileContentLen = getBodyLen(buffer, bufLen);
	char* fileContant = (char*)malloc(sizeof(fileContentLen + 1)); // +1 for '\0'

	getBodyMsg(buffer, bufLen, fileContant, fileContentLen);
	fputs(fileContant, file); // write to file
	free(fileContant);
	fclose(file);
}

int getBodyLen(char* buffer, int bufLen)
{
	for (int i = 0; i < bufLen; i++)
	{
		if (buffer[i] == 'C')
		{
			if (strncmp(buffer + i, "Content-Length: ", 16) == 0)
			{
				i += 16;
				int count = 0;

				while (i < bufLen && isdigit(buffer[i]) != 0)
				{
					i++;
					count++;
				}

				char* num = (char*)malloc(sizeof(count + 1));

				i -= count;
				strncpy(num, buffer + i, count);
				num[count] = '\0';
				int contentLen = atoi(num);

				free(num);
				return contentLen;
			}
		}
	}
}

void getBodyMsg(char* buffer, int bufLen, char* bodyMsg, int bodyLen)
{
	for (int i = 0; i < bufLen - 1; i++)
	{
		if (buffer[i] == '\n' && buffer[i + 1] == '\n')
		{
			strncpy(bodyMsg, buffer + i + 2, bodyLen);
			bodyMsg[bodyLen] = '\0';

			return;
		}
	}
}

char* getFileName(char* buffer)
{

}






FILE* getFilePutReq(char* buffer, int* status, char* statusReq)
{
	FILE* file;
	char* fileName = getFileName(buffer);

	if (file = fopen("fileName", "r+"))
	{
		*status = 200;
		strcpy(statusReq, "OK");
		printf("file exists");
	}
	else
	{
		*status = 201;
		strcpy(statusReq, "Created");
		file = fopen(fileName, "w");
		printf("file doesn't exist");
	}

	return file;
}


void sendMessage(int index, SocketState* sockets)
{
	int bytesSent = 0;
	char sendBuff[255];

	SOCKET msgSocket = sockets[index].id;
	if (sockets[index].sendSubType == GET)
	{
		FILE* file = fopen("English.html", "r");
		if (file == NULL)
		{
			//return FILE_NOT_EXIST;
		}

		char htmlEnglish[256];

		fgets(htmlEnglish, 256, file);
		fclose(file);
		int lenHtml = strlen(htmlEnglish);

		sprintf(sendBuff, "HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: text/html\n\n%s", lenHtml, htmlEnglish);
	}
	else if (sockets[index].sendSubType == PUT)
	{
		int status;
		char statusReq[8];
		FILE* file = getFilePutReq(sockets[index].buffer, &status, statusReq); // = fopen(fileName, "w");

		if (file == NULL)
		{
			sprintf(sendBuff, "HTTP/1.1 501 Not Implemented\nContent-Length: 0\n\n");
		}
		else
		{
			updateFileContent(sockets[index].buffer, file);
			sprintf(sendBuff, "HTTP/1.1 %d %s\nContent-Length: 0\n\n", status, statusReq);
		}
	}
	else if (sockets[index].sendSubType == HEAD)
	{

	}
	else if (sockets[index].sendSubType == POST)
	{
		int bodyLen = getBodyLen(sockets[index].buffer);
		char* bodyMsg = (char*)malloc(sizeof(bodyLen));

		getBodyMsg(sockets[index].buffer, bodyMsg, bodyLen);
		printf("%s", bodyMsg);
	}
	else if (sockets[index].sendSubType == TRACE)
	{

	}
	else if (sockets[index].sendSubType == DELETE)
	{
		char* fileName = getFileName(buffer);

		if (remove(fileName) == 0)
		{
			sprintf(sendBuff, "HTTP/1.1 200 OK\nContent-Length: 0\n\n");

			printf("Deleted successfully");
		}
		else
		{
			sprintf(sendBuff, "HTTP/1.1 204 No Content\nContent-Length: 0\n\n");

			printf("Unable to delete the file");
		}
	}
	else if (sockets[index].sendSubType == OPTIONS)
	{

	}

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	// צריך לבדוק אם בבאפר יש עדיין הודעות אז לעדכן בהתאם
	// לכן זה לא בהכרח IDLE
	sockets[index].send = IDLE; // NOT GOOD~~!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



}
