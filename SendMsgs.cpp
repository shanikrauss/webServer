#define _CRT_SECURE_NO_WARNINGS
#include "SendMsgs.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <winsock2.h>
#include "StructsAndDefines.h"

#define SERVER_NAME "Shani's awesome server"

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

				char* num = (char*)malloc(count + 1);

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

void printPostReq(char* buffer, char** bodyMsg)
{
	int lenLastRecv = strlen(buffer);
	int bodyLen = getBodyLen(buffer, lenLastRecv);
	(*bodyMsg) = (char*)malloc(bodyLen + 1);

	getBodyMsg(buffer, lenLastRecv, (*bodyMsg), bodyLen);
	printf("%s", *bodyMsg);
	
}

void updateSendBuffPostReq(char* sendBuff, char* buffer)
{
	char* bodyMsg = NULL;

	printPostReq(buffer, &bodyMsg);
	int contentLength = strlen(bodyMsg);

	sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nContent-Length: %d\n\n%s", SERVER_NAME, contentLength, bodyMsg);
	free(bodyMsg);
}

void updateFileContent(char* buffer, int bufLen, FILE* file)
{
	int fileContentLen = getBodyLen(buffer, bufLen);
	char* fileContant = (char*)malloc(fileContentLen + 1); // +1 for '\0'

	getBodyMsg(buffer, bufLen, fileContant, fileContentLen);
	fputs(fileContant, file); // write to file
	free(fileContant);
	fclose(file);
}

void getFileName(char* buffer, char** fullFileName)
{
	*fullFileName = (char*)malloc(strlen("C:/temp") + 1);
	strcpy(*fullFileName, "C:/temp");

	//printf("%s/n", *fullFileName);

	//char* fileName = (char*)malloc(2);
	int fileNameSize = 8;
	int curSize = 7;
	int i = 0;

	while (buffer[i] != '\n')
	{
		if (buffer[i] == '/')
		{
			while (strncmp(buffer + i, ".txt", 4) != 0)
			{
				if (curSize == fileNameSize)
				{
					*fullFileName = (char*)realloc(*fullFileName, fileNameSize * 2);
					fileNameSize *= 2;
				}

				(*fullFileName)[curSize] = buffer[i];
				curSize++;
				i++;
			}
			if (curSize + 5 > fileNameSize)
			{
				*fullFileName = (char*)realloc(*fullFileName, fileNameSize * 2);
				fileNameSize *= 2;
			}

			(*fullFileName)[curSize] = '\0';
			printf("%s\n", *fullFileName);

			strcat(*fullFileName, ".txt");
			printf("%s\n", *fullFileName);
			/*int len = strlen(fileName);
			printf("%s\n", fileName);

			int newLen = len + strlen("C:/temp/");
			newLen++; // for \0

			*fullFileName = (char*)realloc(*fullFileName, newLen);
			*fullFileName[]
			printf("%s/n", *fullFileName);

			strcat(*fullFileName, fileName);
			printf("%s", *fullFileName);*/
			return;
		}

		i++;
	}
}


FILE* getFilePutReq(char* buffer, int* status, char* statusReq)
{
	FILE* file;
	char* fileName = NULL;

	getFileName(buffer, &fileName);

	if (file = fopen(fileName, "r+"))
	{
		*status = 200;
		strcpy(statusReq, "OK");
		//printf("file exists");
	}
	else
	{
		*status = 201;
		strcpy(statusReq, "Created");
		file = fopen(fileName, "w");
		//printf("file doesn't exist");
	}

	free(fileName);
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
	*content = (char*)malloc(2);
	int fileNameSize = 2;
	int curSize = 0;
	char ch;

	while (!feof(file))
	{
		fscanf(file, "%c", &ch);

		if (curSize == fileNameSize)
		{
			fileNameSize *= 2;
			*content = (char*)realloc(*content, fileNameSize);
		}

		(*content)[curSize] = ch;
		curSize++;
	}

	fclose(file);

	(*content)[curSize] = '\0';
}




void updateSendBuffGetOrHeadReq(char* sendBuff, char* buffer, int req)
{
	FILE* file = getFileFromBuffer(buffer);

	if (file == NULL) //return FILE_NOT_EXIST; 
	{
		sprintf(sendBuff, "HTTP/1.1 404 Not Found\nServer: %s\nContent-Length: 0\n\n", SERVER_NAME);
		return;
	}

	char* htmlPageContent = NULL;

	getFileContent(file, &htmlPageContent);
	int lenHtml = strlen(htmlPageContent);

	if (req == GET)
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nContent-Length: %d\nContent-Type: text/html\n\n%s", SERVER_NAME, lenHtml, htmlPageContent);
	}
	else // req == HEAD
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nContent-Length: %d\nContent-Type: text/html\n\n",SERVER_NAME, lenHtml);
	}

	free(htmlPageContent);
}

void updateSendBuffHeadReq(char* sendBuff, char* buffer)
{
	updateSendBuffGetOrHeadReq(sendBuff, buffer, HEAD);
}

void updateSendBuffGetReq(char* sendBuff, char* buffer)
{
	updateSendBuffGetOrHeadReq(sendBuff, buffer, GET);
}

void updateSendBuffPutReq(char* sendBuff, char* buffer)
{
	int status;
	char statusReq[8];
	FILE* file = getFilePutReq(buffer, &status, statusReq); // = fopen(fileName, "w");

	if (file == NULL)
	{
		sprintf(sendBuff, "HTTP/1.1 501 Not Implemented\nServer: %s\nContent-Length: 0\n\n", SERVER_NAME);
	}
	else
	{
		updateFileContent(buffer, strlen(buffer), file);
		sprintf(sendBuff, "HTTP/1.1 %d %s\nServer: %s\nContent-Length: 0\n\n", status, statusReq, SERVER_NAME);
	}
}



void updateSendBuffDeleteReq(char* sendBuff, char* buffer)
{
	char* fileName = NULL;
	getFileName(buffer, &fileName);

	if (remove(fileName) == 0)
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nContent-Length: 0\n\n", SERVER_NAME);

		printf("Deleted successfully");
	}
	else
	{
		sprintf(sendBuff, "HTTP/1.1 204 No Content\nServer: %s\nContent-Length: 0\n\n", SERVER_NAME);

		printf("Unable to delete the file");
	}

	free(fileName);
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
		updateSendBuffPutReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == HEAD)
	{
		updateSendBuffHeadReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == POST)
	{
		updateSendBuffPostReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == TRACE)
	{
		int lenReq = strlen(sockets[index].lastRecv);

		sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nContent-Type: message/http\nContent-Length: %d\n\n%s", SERVER_NAME, lenReq, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == DELETE)
	{
		updateSendBuffDeleteReq(sendBuff, sockets[index].lastRecv);
	}
	else if (sockets[index].sendSubType == OPTIONS)
	{
		sprintf(sendBuff, "HTTP/1.1 200 OK\nServer: %s\nAllow: GET,HEAD,POST,PUT,DELETE,OPTIONS,TRACE\n\n", SERVER_NAME);
	}
	else
	{
		sprintf(sendBuff, "HTTP/1.1 404 Not Found\nServer: %s\n\n", SERVER_NAME);
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

