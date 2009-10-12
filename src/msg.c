/*
 *   released under GPL v2
 *
 *    (C) by Ronny T. Lampert (stripped down by Lothar Braun)
 *
 */

#include "msg.h"

#include <stdio.h>
#include <stdarg.h>

static int msg_level=MSG_DEFAULT;
static char *MSG_TAB[]={ "FATAL  ", "TRUMANBOX", "ERROR  ", "DEBUG  ", "INFO   ", 0};

void msg_setlevel(int l)
{
	msg_level = l;
}

/*
 the main logging routine

 we don't have to lock, because glibc's printf() system is doing it already

 if you, however, change this function to a custom backend, you WILL have to lock
 because msg() can and will be called by concurrent threads!
 */
void msg_work(const int line, const char* file, const char* pv, const char* func, const int level, const char *fmt, ...)
{
        /* nummerically higher value means lower priority */
        if (level > msg_level) {
                return;
        } else {
                va_list args;
                printf("%s in %s:%d: ", MSG_TAB[level], file, line);
                va_start(args, fmt);
                vprintf(fmt, args);
                va_end(args);
                printf("\n");
        }
}

