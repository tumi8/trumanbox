#include "configuration.h"
#include "iniparser.h"
#include "msg.h"

#include <stdlib.h>


struct configuration_t {
	dictionary* dict;
};

struct configuration_t* configuration_create(const char* config_file)
{
	struct configuration_t* ret = (struct configuration_t*)malloc(sizeof(struct configuration_t));
	ret->dict = iniparser_new(config_file);
	if (ret->dict == NULL) {
		msg(MSG_ERROR, "Error parsing configuration file!");
		goto out1;
	}
	return ret;
out1:
	free(ret);
	return NULL;
}

void configuration_destroy(struct configuration_t* c)
{
	iniparser_free(c->dict);
	free(c);
}

const char* configuration_getvalue(struct configuration_t* c, const char* module, const char* key)
{
	return iniparser_getvalue(c->dict, module, key);
}
