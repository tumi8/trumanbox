#include "configuration.h"
#include "iniparser.h"
#include "msg.h"

#include <stdlib.h>


struct configuration_t {
	dictionary* dict;
	operation_mode_t mode;
};

struct configuration_t* conf_create(const char* config_file)
{
	struct configuration_t* ret = (struct configuration_t*)malloc(sizeof(struct configuration_t));
	ret->dict = iniparser_new(config_file);
	ret->mode = invalid;
	if (ret->dict == NULL) {
		msg(MSG_ERROR, "Error parsing configuration file!");
		goto out1;
	}
	return ret;
out1:
	free(ret);
	return NULL;
}

void conf_destroy(struct configuration_t* c)
{
	iniparser_free(c->dict);
	free(c);
}

const char* conf_get(struct configuration_t* c, const char* module, const char* key)
{
	return iniparser_getvalue(c->dict, module, key);
}

int conf_getint(struct configuration_t* c, const char* module, const char* key, int notfound)
{
	const char* tmp = iniparser_getvalue(c->dict, module, key);
	if (tmp == NULL) {
		return notfound;
	}
	return atoi(tmp);
}

operation_mode_t conf_get_mode(struct configuration_t* c)
{
	operation_mode_t m = invalid;
	if (-1 == (m = conf_getint(c, "main", "mode", -1))) {
		return c->mode;
	}
	if (m > invalid && m < quit) {
		return c->mode;
	}
	return invalid;
}

int conf_set_mode(struct configuration_t* c, operation_mode_t mode)
{
	if (-1 == conf_getint(c, "main", "mode", -1)) {
		// there is no mode in the configuration file
		if (mode > invalid && mode < quit) {
			c->mode = mode;
			return 0;
		}
		msg(MSG_ERROR, "Invalid mode given: %d", mode);
		return -1;
	} else {
		msg(MSG_ERROR, "Mode given in configuration file. Cannot overwrite this value!");
	}
	return -1;
}

