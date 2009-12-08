#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdint.h>
#include "definitions.h"

struct configuration_t;

/* Creates a configuration object by parsing the given ini file
 * @returns a configuration object on sucess, NULL otherwise
 */
struct configuration_t* conf_create(const char* config_file);

void conf_destroy(struct configuration_t* c);

const char* conf_get(struct configuration_t* c, const char* module, const char* key);
int conf_getint(struct configuration_t* c, const char* module, const char* key, int notfound); 

/*
 * Returns the application mode. The return value depends on different program state:
 * If the mode was specified in the configuration file, this mode will be returned
 * If no mode was given in the config file, INVALID will be returned as long as there 
 * was no mode set with @conf_set_mode
 */
operation_mode_t conf_get_mode(struct configuration_t* c);

/* Sets the application run mode, if there was no mode set in the config file.
 * This function will return an error if there was a valid mode in the configuration.
 * @return 0 on success, 1 on error
 */
int conf_set_mode(struct configuration_t* c, operation_mode_t mode);


#endif
