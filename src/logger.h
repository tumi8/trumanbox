#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "definitions.h"

struct logger_t;
struct configuration_t;

typedef int (lg_init)(struct logger_t*);
typedef int (lg_log)(struct logger_t*);

enum logger_type { directory };

struct logger_t {
	void* logger_data;
	struct configuration_t* config;

	lg_init* init;
	lg_log* log;	
};

struct logger_t* logger_create(struct configuration_t* config);
void logger_destroy(struct logger_t* logger);

#endif
