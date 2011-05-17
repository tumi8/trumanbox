#include "helper_file.h"
#include "dns_resolver.h"
#include "dispatching.h"
#include "semaphore.h"
#include "logger.h"

#include <common/msg.h>
#include <common/configuration.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static operation_mode_t interactive_menu();
static void usage(const char* progname);
static operation_mode_t get_mode(const char* mode_string);

int main(int argc, char **argv) {
	const char *workdir;
	int		c;
	operation_mode_t mode = invalid;
	struct dns_resolver_t* dns_resolver;
	struct dispatcher_t* dispatcher;
	int msg_level = MSG_ERROR;
	char* config_file = NULL;
	

	while (-1 != (c=getopt(argc, argv, "hdf:"))) {
		switch (c) {
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

	Configuration config(config_file);

	msg(MSG_DEBUG, "Trumanbox is running in mode %d", mode);

	semaph_init();

	// change to working dir
	workdir = config.get("main", "work_dir");
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

	msg(MSG_DEBUG, "Trumanbox is quitting...");

	exit(0);
}


void usage(const char* progname)
{
	printf("Usage: %s [-h] [-d] -f configfile\n", progname);
}


