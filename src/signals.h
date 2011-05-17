#ifndef _SIGNAL_H__
#define _SIGNAL_H__

#ifndef SIGFUNC_H
#define SIGFUNC_H
typedef void Sigfunc(int);
#endif

Sigfunc *signal_wrapper(int signo, Sigfunc *func);
Sigfunc *Signal(int signo, Sigfunc *func);
void sig_chld(int signo);

#endif
