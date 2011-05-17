#include "configuration.h"
#include "msg.h"

#include <stdlib.h>


Configuration::Configuration(const std::string& filename)
{
	this->dict = iniparser_new(filename.c_str());
	if (this->dict == NULL) {
		THROWEXCEPTION("Error parsing configuration file!");
	}

	const char* tmp = iniparser_getvalue(this->dict, "main", "mode");
	if (!tmp) {
		THROWEXCEPTION("TrumanBox Mode not defined in section \"main\" in configuration file!");
	}
	this->mode = (operation_mode_t)atoi(tmp);
}

Configuration::~Configuration()
{
	iniparser_free(this->dict);
}

std::string Configuration::get(const std::string& section, const std::string& key, const std::string& def) const
{
	const char* tmp = iniparser_getvalue(this->dict, section.c_str(), key.c_str());
	if (!tmp) {
		return def;
	}
	return tmp;
}

int Configuration::getInt(const std::string& section, const std::string& key, int def) const
{
	const char* tmp = iniparser_getvalue(this->dict, section.c_str(), key.c_str());
	if (!tmp) {
		return def;
	}
	return atoi(tmp);
}

operation_mode_t Configuration::getMode() const
{
	return this->mode;
}


