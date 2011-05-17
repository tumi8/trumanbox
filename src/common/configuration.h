#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdint.h>
#include <string>
#include "definitions.h"
#include "iniparser.h"

/**
 * Global class which holds all necessary configuration items, parsed from the
 * ini file that has been given on the command line
 * This class is accessed and shared ober all other modules within TrumanBox
 */
class Configuration
{
	public:
		Configuration(const std::string& filename);
		~Configuration();

		std::string get(const std::string& section, const std::string& key, const std::string& def = "") const;
		int getInt(const std::string& section, const std::string& key, int def = 0) const;

		operation_mode_t getMode() const;

	private:
		operation_mode_t mode;
		dictionary* dict;
};

#endif
