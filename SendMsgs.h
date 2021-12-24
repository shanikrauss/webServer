#ifndef SENDMSGS_H
#define SENDMSGS_H

#include <iostream>
#include "StructsAndDefines.h"

int getBodyLen(char* buffer, int bufLen);

void getBodyMsg(char* buffer, int bufLen, char* bodyMsg, int bodyLen);

void printPostReq(char* buffer, char** pBodyMsg);

void updateSendBuffPostReq(char* sendBuff, char* buffer);

void updateFileContent(char* buffer, int bufLen, FILE* file);

void getFileName(char* buffer, char** fullFileName);

FILE* getFilePutReq(char* buffer, int* status, char* statusReq);

FILE* getFileFromBuffer(char* buffer);

void getFileContent(FILE* file, char** content);

void updateSendBuffGetOrHeadReq(char* sendBuff, char* buffer, int req);

void updateSendBuffHeadReq(char* sendBuff, char* buffer);

void updateSendBuffGetReq(char* sendBuff, char* buffer);

void updateSendBuffPutReq(char* sendBuff, char* buffer);

void updateSendBuffDeleteReq(char* sendBuff, char* buffer);

void sendMessage(SocketState* sockets, int index);


#endif
