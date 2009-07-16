/*
 * here comes the first version of the truman-box working in 4 different modes:
 *	- simulation
 *	- half-proxy
 *	- full-proxy
 *	- transparent
 *
 * - dispatcher and interception are both driven by the same process
 * - in the modes simulation, half-proxy and full-proxy a new process
 *   is forked for every new connection accepted by the dispatcher
 * - every connection is used bi-directional
 * - so far we only support tcp/ip
 *
 * this version does (in half-proxy mode):
 *  IRC:
 *    - logging
 *  FTP:
 *    - logging
 *    - payload alteration
 *      + replaying original banner, if possible
 *      + altering ftp passive response
 *    - triggering other functions:
 *	+ build filesystem structure on demand
 *  HTTP:
 *    - payload alteration:
 *      + remove encoding flag
 *    - triggering other functions:
 *	+ build filesystem structure on demand
 *	+ create index file in every created path
 *  SMTP:
 *    - payload alteration:
 *	+ replaying original banner, if possible
 *
 *
 * IMPORTANT:
 * for the transparent mode it might be necessary to apply more memspace
 * by altering the proc filesystem...
 *
 * TODO:
 * in content substitution and logging are unnecessary string copy functions...
 *
 */

#include "main.h"

typedef enum {
	invalid,
	full_emulation,
	half_proxy,
	full_proxy,
	quit
} operation_mode_t;

static operation_mode_t interactive_menu();
static void usage(const char* progname);

int main(int argc, char **argv) {
	char		config_dir[256]; //, config_cmd[256];
	int		c;
	operation_mode_t mode = invalid;

	if (argc == 1) {
		// no parameters given, go into interactive mode
		do {
			mode = interactive_menu();
		} while (mode == invalid);
		if (mode == quit) {
			printf("TrumanBox is quitting ...\n");
			return 0;
		}
	} else {
		while (-1 != (c=getopt(argc, argv, "hm:"))) {
			switch (c) {
			case 'm':
				mode = atoi(optarg);
				if (mode < full_emulation || mode > full_proxy) {
					usage(argv[0]);
					exit(-1);
				}
				break;
			case 'h':
			default:
				usage(argv[0]);
				exit(1);
			}
		}
	}

	create_tmp_folders();
	strncpy(config_dir, TRUMAN_CONFIG_DIR, 255);
	change_to_tmp_folder();

	semaph_init();

	dispatching(mode);

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
		\n");

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
	printf("Usage: %s [-h] [-m <mode>]\n"
	       "\tAvailable modes:\n"
	       "\t\t1 - full emulation\n"
	       "\t\t2 - half proxy\n"
	       "\t\t3 - full proxy\n", progname);
}
