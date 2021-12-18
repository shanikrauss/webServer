#ifndef STRUCTSANDDEFINES_H
#define STRUCTSANDDEFINES_H
#include <winsock2.h>

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving? האם זה מישהו שאנחנו אמורים לקבל ממנו מידע כלומר עברנו את שלב הקליטה עשינו אקספט ועכשיו אנחנו מחכים ממנו להודעה
	int	send;			// Sending? האם אנחנו בשלב של שליחת תשובה
	int sendSubType;	// Sending sub-type איזה תשובה אם אנחנו בשלב של שליחת תשובה
	char buffer[128]; // שומר את הבקשות כי תשובה אנחנו שולחים ישר
	int len;
};

#define TIME_PORT = 27015;
#define MAX_SOCKETS = 60; // כמה לקוחות לקבל במקביל
#define EMPTY = 0; // לא מקבל שום דבר
#define LISTEN = 1; // מקבל אבל הוא אמור לקבל בקשות לקשר
#define RECEIVE = 2; // מחכה לקבל הודעות דאטה
#define IDLE = 3; // בשביל הסנד - כאן אין מה לשלוח
#define SEND = 4; // בשביל הסנד - יש מה לשלוח
//const int SEND_TIME = 1; // איזה הודעה אני הולך לשלוח בשביל קריאות הקוד
//const int SEND_SECONDS = 2; //איזה הודעה אני הולך לשלוח בשביל קריאות הקוד

#define GET 1
#define PUT 2
#define HEAD 3
#define POST 4
#define TRACE 5
#define DELETE 6
#define OPTIONS 7

#endif
