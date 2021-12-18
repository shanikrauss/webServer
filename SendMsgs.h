#ifndef SENDMSGS_H
#define SENDMSGS_H

#include <iostream>


void updateFileContent(char* buffer, FILE* file);

FILE* getFilePutReq(char* buffer, int* status, char* statusReq);

void sendMessage(int index);

#endif
