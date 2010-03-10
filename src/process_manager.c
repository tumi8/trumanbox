#include "process_manager.h"
#include "msg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_PROCESS 100


// Process list elements
struct pl_element {
	pid_t pid;
	struct pl_element* prev;
	struct pl_element* next;
	int restart_flag; // stores whether a process needs to be killed on pm_kill_temporary
};

struct process_manager_t {
	struct pl_element* processes;
};

static int pm_initialized = 0;
static struct process_manager_t* pm = NULL;

void pm_init(void)
{
	if (pm_initialized) {
		msg(MSG_FATAL, "pm_init called but process manager is already initialized");
		exit(-1);
	}
	pm = (struct process_manager_t*)malloc(sizeof(struct process_manager_t));
	if (!pm) {
		msg(MSG_FATAL, "Could not initialize process manager: %s", strerror(errno));
		exit(-1);
	}
	pm->processes = NULL;
	pm_initialized = 1;
}

void pm_destroy(void)
{
	if (pm_initialized) {
		free(pm);
		pm_initialized = 0;
	} else {
		msg(MSG_FATAL, "pm_destroy called but process manager was not called before");
		exit(-1);
	}
}

void pm_add(struct process_manager_t* pm, pid_t child_pid, int restart)
{
	struct pl_element* i = pm->processes;
	if (!i) {
		// first element
		struct pl_element* ple = (struct pl_element*)malloc(sizeof(struct pl_element));
		ple->prev = NULL;
		ple->next = NULL;
		ple->pid = child_pid;		
		ple->restart_flag = restart;
		pm->processes = ple;
		return;
	}

	while (i->next)
		i = i->next;

	struct pl_element* ple = (struct pl_element*)malloc(sizeof(struct pl_element));
	ple->pid = child_pid;
	ple->prev = i;
	ple->next = NULL;
	ple->restart_flag = restart;
	i->next = ple;
}

int pm_del(struct process_manager_t* pm, pid_t child_pid)
{
	struct pl_element* i = pm->processes;
	if (!i) {
		msg(MSG_FATAL, "pm_del: No processes stored!");
		exit(-1);
	}

	while (i->pid != child_pid && i->next)
		i = i->next;

	if (i->pid != child_pid) {
		msg(MSG_FATAL, "pm_del: PID %d is unknown to process manager");
		exit(-1);
	}

	if (!i->prev) {
		// i is last element in the list
		free(i);
		pm->processes = NULL;
	}

	if (i->prev)
		i->prev->next = i->next;
	if (i->next)
		i->next->prev = i->prev;
	free(i);
	return 0;
}

static pid_t Fork(int restart) {
	pid_t	pid;

	if (!pm_initialized) {
		msg(MSG_FATAL, "Process manager is not initialized! Call pm_init() before doing your first Fork()!!!!");
		exit(-1);
	}	

	if ( (pid = fork()) == -1) {
		msg(MSG_ERROR, "fork error: %s", strerror(errno));
	} else if (pid != 0) {
		pm_add(pm, pid, restart);
	}

	return(pid);
}

pid_t pm_fork_permanent(void)
{
	return Fork(0);
}

pid_t pm_fork_temporary(void)
{
	return Fork(1);
}


void pm_kill_temporary(void)
{
	struct pl_element* ple = pm->processes;
	while (ple) {
		if (ple->restart_flag) {
			kill(ple->pid, SIGINT);
			pm_del(pm, ple->pid); // TODO: this is stupid. optimize!
		}
		ple = ple->next;
	}
}

