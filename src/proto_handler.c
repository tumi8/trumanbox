#include "proto_handler.h"
#include "definitions.h"
#include "msg.h"

#include <stdlib.h>

static struct protohandler_t* create_handler(protocols_app app, struct configuration_t* c)
{
	struct protohandler_t* ret = NULL;
	switch (app) {
		default:
			msg(MSG_FATAL, "No handler for protocol %d defined! This is a bug (and will result in segmentation faults! Aborting!", app);
			exit(-1);
	}
	return ret;
}


struct protohandler_t** ph_create(struct configuration_t* c)
{
	struct protohandler_t** result = (struct protohandler_t**)malloc(sizeof(struct protohandler_t*)*UNKNOWN);
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		result[i] = create_handler(i, c);
	}

	return result;
}


int ph_destroy(struct protohandler_t** p)
{
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		free(p[i]);
	}
	free(p);
	return 0;
}

