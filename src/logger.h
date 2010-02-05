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

/**
 * Note: Every logging module MUST expect concurrent access to the logging
 * data by several *processes*, as the trumanbox forks several processes 
 * in order to do its jobs.
 */
struct logger_t {
	void* data;
	struct configuration_t* config;

	lg_init* init; // will be called by logger_create
	lg_deinit* deinit; // will be called by logger_destroy
	lg_create_log* create_log;
	lg_finish_log* finish_log;
	lg_log_text* log;
};


/**
 * This creates the logging object. There may be an arbitrary number of logging objets
 * The logging module *MUST* expect that different *processes* want concurrent access to
 * the logs. 
 */
struct logger_t* logger_create(struct configuration_t* config);

void logger_destroy(struct logger_t*);

#endif
