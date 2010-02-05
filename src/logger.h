#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "definitions.h"

struct logger_t;
struct configuration_t;

typedef int (lg_init)(struct logger_t*); // initializes the logging module. Returns 0 on sucess, a value < 0 otherwise
typedef int (lg_deinit)(struct logger_t*);// cleans up the logger module. Returns 0 on sucess, a value < 0 otherwise
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
 * The logging module *MUST* expect that different *processes* and modules want concurrent 
 * access to the logs. 
 */
int logger_create(struct configuration_t* config);

/**
 * Get the global logger object. logger_create MUST be called once before 
 * this function is called.
 */
inline struct logger_t* logger_get();

int logger_destroy();

#endif