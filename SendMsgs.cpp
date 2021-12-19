#define _CRT_SECURE_NO_WARNINGS
#include "SendMsgs.h"
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
	int i = bufLen - bodyLen;
	strncpy(bodyMsg, buffer + i, bodyLen);
	bodyMsg[bodyLen] = '\0';
	/*
	for (int i = 0; i < bufLen - 1; i++)
	{
		if (buffer[i] == '\r' && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' && buffer[i + 3] == '\n')
		{
			strncpy(bodyMsg, buffer + i + 2, bodyLen);
			bodyMsg[bodyLen] = '\0';

			return;
		}
	}*/
}

char* getFileName(char* buffer)
{
	char a[2] = {"A"};
	return a;
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

FILE* getFileFromBuffer(char* buffer)
{
	char fileName[30] = { "C:/temp/English.txt" };
	int i = 0;

	while (buffer[i] != '\0' && buffer[i] != '\n')
	{
		if (strncmp(buffer + i, "lang=he", 7) == 0)
		{
			strcpy(fileName, "C:/temp/Hebrew.txt");
			break;
		}
		else if (strncmp(buffer + i, "lang=fr", 7) == 0)
		{
			strcpy(fileName, "C:/temp/French.txt");
			break;
		}

		i++;
	}

	FILE* file = fopen(fileName, "r");

	return file;
}

void getFileContent(FILE* file, char** content)
{
	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	*content = (char*)malloc(sizeof(fileSize));

	//fgets(htmlEnglish, fileSize + 1, file);
	//fread(htmlEnglish, sizeof(char), fileSize + 1, file);
	fread((*content), sizeof(char), fileSize, file);
	fclose(file);

	(*content)[fileSize - 1] = '\0';
}

void updateSendBuffGetOrHeadReq(char* sendBuff, char* buffer, int req)
{
	FILE* file = getFileFromBuffer(buffer);

	if (file == NULL) //return FILE_NOT_EXIST; 
	{
		sprintf(sendBuff, "HTTP/1.1 404 Not Found\nContent-Length: 0\n\n");
		return;
	}

	char* htmlPageContent = NULL;

	getFileContent(file, &htmlPageContent);
	int lenHtml = strlen(htmlPageContent);

	if (req == GET)
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: text/html\n\n%s", lenHtml, htmlPageContent);
	}
	else // req == HEAD
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: text/html\n\n", lenHtml);
	}

	//free(htmlPageContent);
}

void updateSendBuffHeadReq(char* sendBuff, char* buffer)
{
	updateSendBuffGetOrHeadReq(sendBuff, buffer, HEAD);
}

void updateSendBuffGetReq(char* sendBuff, char* buffer)
{
	updateSendBuffGetOrHeadReq(sendBuff, buffer, GET);
}

void printPostReq(char* buffer, char** bodyMsg)
{
	int lenLastRecv = strlen(buffer);
	int bodyLen = getBodyLen(buffer, lenLastRecv);
	(*bodyMsg) = (char*)malloc(sizeof(bodyLen+1));

	getBodyMsg(buffer, lenLastRecv, (*bodyMsg), bodyLen);
	printf("%s", *bodyMsg);
}

void sendMessage(int index, SocketState* sockets)
{
	int bytesSent = 0;
	char sendBuff[255];
	SOCKET msgSocket = sockets[index].id;

	if (sockets[index].sendSubType == GET)
	{
		updateSendBuffGetReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == PUT)
	{
		int status;
		char statusReq[8];
		FILE* file = getFilePutReq(sockets[index].lastRecv, &status, statusReq); // = fopen(fileName, "w");

		if (file == NULL)
		{
			sprintf(sendBuff, "HTTP/1.1 501 Not Implemented\nContent-Length: 0\n\n");
		}
		else
		{
			updateFileContent(sockets[index].buffer[0], sockets[index].bufferLen[0], file);
			sprintf(sendBuff, "HTTP/1.1 %d %s\nContent-Length: 0\n\n", status, statusReq);
		}
	}
	else if (sockets[index].sendSubType == HEAD)
	{
		updateSendBuffHeadReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == POST)
	{
		char* bodyMsg = NULL;

		printPostReq(sockets[index].lastRecv, &bodyMsg);
		int contentLength = strlen(bodyMsg);

		sprintf(sendBuff, "HTTP/1.1 200 OK\nContent-Length: %d\n\n%s", contentLength, bodyMsg);
		//strcpy(sendBuff, bodyMsg); // needed?

		//free(bodyMsg);
	}
	else if (sockets[index].sendSubType == TRACE)
	{

	}
	else if (sockets[index].sendSubType == DELETE)
	{
		char* fileName = getFileName(sockets[index].buffer[0]);

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

