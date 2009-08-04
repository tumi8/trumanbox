#ifndef _MSG_H_
#define _MSG_H_

#define MSG_BLANK 256
#define MSG_INFO 4
#define MSG_DEBUG 3
#define MSG_ERROR 2
#define MSG_DIALOG 1
#define MSG_FATAL 0
#define MSG_DEFAULT MSG_ERROR

void msg(int, char *, ...);

#endif
