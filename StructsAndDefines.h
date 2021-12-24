#ifndef STRUCTSANDDEFINES_H
#define STRUCTSANDDEFINES_H
#include <winsock2.h>

#define WEB_PORT 8080
#define MAX_SOCKETS 60 // כמה לקוחות לקבל במקביל
#define MAX_SIZE_REQUEST 500
#define MAX_REQUESTS_PER_SOCKET 10
#define EMPTY 0 // לא מקבל שום דבר
#define LISTEN 1 // מקבל אבל הוא אמור לקבל בקשות לקשר
#define RECEIVE 2 // מחכה לקבל הודעות דאטה
#define IDLE 3 // בשביל הסנד - כאן אין מה לשלוח
#define SEND 4 // בשביל הסנד - יש מה לשלוח
//const int SEND_TIME = 1; // איזה הודעה אני הולך לשלוח בשביל קריאות הקוד
//const int SEND_SECONDS = 2; //איזה הודעה אני הולך לשלוח בשביל קריאות הקוד

#define GET 1
#define PUT 2
#define HEAD 3
#define POST 4
#define TRACE 5
#define DELETE 6
#define OPTIONS 7
#define UNKNOWN_REQ -1

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving? האם זה מישהו שאנחנו אמורים לקבל ממנו מידע כלומר עברנו את שלב הקליטה עשינו אקספט ועכשיו אנחנו מחכים ממנו להודעה
	int	send;			// Sending? האם אנחנו בשלב של שליחת תשובה
	int sendSubType;	// Sending sub-type איזה תשובה אם אנחנו בשלב של שליחת תשובה
	char lastRecv[MAX_SIZE_REQUEST]; // מערך של הבקשה האחרונה
	char buffer[MAX_REQUESTS_PER_SOCKET][MAX_SIZE_REQUEST]; // שומר את הבקשות כי תשובה אנחנו שולחים ישר
	int bufferLen[MAX_REQUESTS_PER_SOCKET];
	int avilable;
	time_t timeOfLastReq;

}; typedef struct SocketState SocketState;

#endif
