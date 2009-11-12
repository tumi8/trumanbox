#ifndef __PROCESS_MANAGER_H__
#define __PROCESS_MANAGER_H__

#include <unistd.h>

void pm_init(void);
void pm_destroy(void);

pid_t Fork(void);

#endif
