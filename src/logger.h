#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "definitions.h"

struct logger_t;
struct configuration_t;

typedef int (lg_init)(struct logger_t*);
typedef int (lg_deinit)(struct logger_t*);
typedef int (lg_create_log)(struct logger_t*);
typedef int (lg_finish_log)(struct logger_t*);
typedef int (lg_log_text)(struct logger_t*, char* fmt, ...);

enum logger_type { directory };

struct logger_t {
	void* logger_data;
	struct configuration_t* config;

	lg_init* init;
	lg_create_log* create_log;
	lg_finish_log* finish_log;
	lg_log_text* log;
};

struct logger_t* logger_create(struct configuration_t* config);
void logger_destroy(struct logger_t* logger);

#endif
