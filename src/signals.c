#include "signals.h"
#include "msg.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

Sigfunc *signal(int signo, Sigfunc *func) {
        struct sigaction        act, oact;

        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        if (signo == SIGALRM) {
#ifdef  SA_INTERRUPT
                act.sa_flags |= SA_INTERRUPT;   /* SunOS 4.x */
#endif
        } else {
#ifdef  SA_RESTART
                act.sa_flags |= SA_RESTART;             /* SVR4, 44BSD */
#endif
        }
        if (sigaction(signo, &act, &oact) < 0)
                return(SIG_ERR);
        return(oact.sa_handler);
}
/* end signal */

Sigfunc *Signal(int signo, Sigfunc *func) {       /* for our signal() function */

        Sigfunc *sigfunc;

        if ( (sigfunc = signal(signo, func)) == SIG_ERR)
                msg(MSG_ERROR, "signal error");
        return(sigfunc);
}

void sig_chld(int signo) {
	pid_t pid;
	int   stat;
	
	while( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		msg(MSG_DEBUG, "child %d terminated %s", pid, WIFEXITED(stat)?"normally":"abnormally");
		if (WIFEXITED(stat))
			msg(MSG_DEBUG, "\tExit code was %d", WEXITSTATUS(stat));
		if (WIFSIGNALED(stat)) 
			msg(MSG_DEBUG, "\tProcess was signaled with signal %d", WTERMSIG(stat));
	}
	return;
}

