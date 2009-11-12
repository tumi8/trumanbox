#ifndef __PROCESS_MANAGER_H__
#define __PROCESS_MANAGER_H__

#include <unistd.h>

void pm_init(void);
void pm_destroy(void);

pid_t pm_fork_permanent(void);
pid_t pm_fork_temporary(void);

void pm_kill_temporary(void);

#endif
