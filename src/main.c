#include "msg.h"
#include "configuration.h"
#include "helper_file.h"
#include "dns_resolver.h"
#include "dispatching.h"
#include "semaphore.h"
#include "logger.h"


#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static operation_mode_t interactive_menu();
static void usage(const char* progname);
static operation_mode_t get_mode(const char* mode_string);

int main(int argc, char **argv) {
	const char *config_dir, *workdir;
	int		c;
	operation_mode_t mode = invalid;
	struct dns_resolver_t* dns_resolver;
	struct dispatcher_t* dispatcher;
	int msg_level = MSG_ERROR;
	char* config_file = NULL;
	

	while (-1 != (c=getopt(argc, argv, "hdm:f:"))) {
		switch (c) {
		case 'm':
			mode = get_mode(optarg);
			if (mode == invalid) {
				usage(argv[0]);
				exit(-1);
			}
			break;
		case 'd':
			msg_level++;
			break;
		case 'f':
			config_file=optarg;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(1);
		}
	}
	msg_setlevel(msg_level);

	if (!config_file) {
		msg(MSG_FATAL, "No config file given!");
		usage(argv[0]);
		exit(2);
	}

	struct configuration_t* config = conf_create(config_file);
	if (!config) {
		msg(MSG_FATAL, "No valid configuration given. Exiting!");
		exit(-3);
	}

	if (mode == invalid) {
		// no mode given, check if config file specifies a mode
		mode = conf_get_mode(config);
		if (mode == invalid) {
			msg(MSG_DEBUG, "No valid operation mode in config file %s. Starting interactive mode...");
			do {
				mode = interactive_menu();
			} while (mode == invalid);
		}
		if (mode == quit) {
			msg(MSG_INFO, "TrumanBox is quitting ...");
			return 0;
		}
		conf_set_mode(config, mode);
	}

	msg(MSG_DEBUG, "Trumanbox is running in mode %d", mode);

	config_dir = conf_get(config, "main", "config_dir");

	semaph_init();

	// change to working dir
	workdir = conf_get(config, "main", "work_dir");
  	if (chdir(workdir) < 0) {
		msg(MSG_FATAL, "cannot change working dir to %s: %s", workdir, strerror(errno));
		return -1;
	}


	if (0 > logger_create(config)) {
		msg(MSG_FATAL, "Failure while initializing logging module. Exiting ...");
		return -1;
	}

	dns_resolver = dns_create_resolver(config);
	dispatcher = disp_create(config);

	msg(MSG_DEBUG, "Running dns resolver");
	dns_start_resolver(dns_resolver);
	msg(MSG_DEBUG, "Running dispatcher");
	disp_run(dispatcher);
	msg(MSG_DEBUG, "Stopping dns resolver");
	dns_stop_resolver(dns_resolver);
	dns_destroy_resolver(dns_resolver);
	disp_destroy(dispatcher);
	logger_destroy();

	conf_destroy(config);
	msg(MSG_DEBUG, "Trumanbox is quitting...");

	exit(0);
}

/** Displays an interactive menu that allows to choose the operation mode.
 * @return Chosen operation
 */
static operation_mode_t  interactive_menu()
{
	int choice;

	printf("\n\n\n\t\t\t\t=- welcome to the TrumanBox -=\n \
		\n \
		please be aware of the fact that this program will delete\n \
		all iptable rules currently running. in the same time rulesets\n \
		needed to run the TrumanBox will be applied.\n \
		\n \
		if you decide to exit the program now, the current iptable \n \
		settings will stay untouched\n \
		\n \
		otherwise you can just decide in which mode you wish to run the\n \
 		TrumanBox:\n \
		\n \
		\t(1) full emulation\n \
		\t(2) half proxy\n \
		\t(3) full proxy\n \
		\t(q) quit\n \
		\n \
		Your choice: ");

	choice = getchar();

	if (choice  == 'q')
		return quit;

	choice -= '0';

	if (choice < 1 || choice > 3) {
		return invalid;
	}

	return choice;
}

void usage(const char* progname)
{
	printf("Usage: %s [-h] [-d] [-m <mode>] -f configfile\n"
	       "\tAvailable modes:\n"
	       "\t\t1 - full emulation\n"
	       "\t\t2 - half proxy\n"
	       "\t\t3 - full proxy\n", progname);
}

static operation_mode_t get_mode(const char* mode_string)
{
	if (!mode_string) {
		msg(MSG_ERROR, "No mode string given!");
		return invalid;
	}
	operation_mode_t mode =	atoi(mode_string);
	if (mode < full_emulation || mode > full_proxy) {
		mode = invalid;
	}
	return mode;
}
