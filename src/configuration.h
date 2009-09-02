#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdint.h>

struct configuration_t;

struct configuration_t* configuration_create(const char* config_file);
void configuration_destroy(struct configuration_t* c);

#endif
