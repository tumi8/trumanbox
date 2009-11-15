#ifndef _DNS_RESOLVER_H_
#define _DNS_RESOLVER_H_

#include <stdint.h>
#include "definitions.h"

struct dns_resolver_t;
struct configuration_t;

struct dns_resolver_t* dns_create_resolver(struct configuration_t* c);

void dns_start_resolver(struct dns_resolver_t* r);
void dns_stop_resolver(struct dns_resolver_t* r);

void dns_destroy_resolver(struct dns_resolver_t* r);

#endif
