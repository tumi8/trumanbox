#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdint.h>

struct configuration_t;

struct configuration_t* conf_create(const char* config_file);
void conf_destroy(struct configuration_t* c);
const char* conf_get(struct configuration_t* c, const char* module, const char* key);
int conf_getint(struct configuration_t* c, const char* module, const char* key, int notfound); 

#endif
