#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include "definitions.h"

struct dispatcher_t;
struct configuration_t;

struct dispatcher_t* disp_create(struct configuration_t* c, operation_mode_t mode);
int disp_destroy(struct dispatcher_t* d);

void disp_run(struct dispatcher_t* d);

#endif
