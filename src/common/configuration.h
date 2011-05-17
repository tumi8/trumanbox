#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdint.h>
#include <string>
#include "definitions.h"
#include "iniparser.h"

class Configuration
{
	public:
		Configuration(const std::string& filename);
		~Configuration();

		std::string get(const std::string& section, const std::string& key);

		operation_mode_t getMode();

	private:
		operation_mode_t mode;
		dictionary* dict;
};

#endif
