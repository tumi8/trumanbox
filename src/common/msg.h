#ifndef _MSG_H_
#define _MSG_H_

#define MSG_BLANK 256
#define MSG_INFO 4
#define MSG_DEBUG 3
#define MSG_ERROR 2
#define MSG_DIALOG 1
#define MSG_FATAL 0
#define MSG_DEFAULT MSG_ERROR

#define msg(lvl, fmt, args...) msg_work(__LINE__, __FILE__, __PRETTY_FUNCTION__, __func__, lvl, fmt, ##args)

void msg_setlevel(int l);
void msg_work(const int, const char*, const char*, const char*, const int, const char *, ...);

#endif
