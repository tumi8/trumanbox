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


int main(int argc, char **argv) {
	int		choice;
	char		config_dir[256], config_cmd[256];

	create_tmp_folders();

	strncpy(config_dir, TRUMAN_CONFIG_DIR, 255);

	change_to_tmp_folder();

	semaph_init();

print_menu:
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
		exit(0);

	choice -= '0';

	if (choice < 1 || choice > 3)
		goto print_menu;

	printf("you decided to run the TrumanBox in mode: %d\n", choice);
/*	
	sprintf(config_cmd, "%s/clean_iptables.sh", TRUMAN_CONFIG_DIR);
	printf("deleting existing iptable rules...\n");
	system(config_cmd);
	printf("...successfull!\n");
*/	
/*
	if (choice == 4) {
		sprintf(config_cmd, "%s/netfilter_setup_transparent.sh", TRUMAN_CONFIG_DIR);
		printf("setting up iptable rules for transparent mode...\n");
		system(config_cmd);
		printf("...successfull!\n");
		interception_transparent();
	}
	else {
		sprintf(config_cmd, "%s/netfilter_setup.sh", TRUMAN_CONFIG_DIR);
		printf("setting up iptable rules for mode %d ...\n", choice);
		system(config_cmd);
		printf("...successfull!\n");
*/
		dispatching(choice);
//	}
	exit(0);
}



