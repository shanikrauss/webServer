#ifndef SENDMSGS_H
#define SENDMSGS_H

#include <iostream>

void updateFileContent(char* buffer, int bufLen, FILE* file);

int getBodyLen(char* buffer, int bufLen);

void getBodyMsg(char* buffer, int bufLen, char* bodyMsg, int bodyLen);

char* getFileName(char* buffer);

FILE* getFilePutReq(char* buffer, int* status, char* statusReq);

void sendMessage(int index, SocketState* sockets);

#endif
