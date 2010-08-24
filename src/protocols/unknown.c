#include "unknown.h"
#include "wrapper.h"
#include "helper_file.h"
#include "msg.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

struct ph_unknown {
	struct configuration_t* config;
};

void* ph_unknown_create()
{
	void* ret = malloc(sizeof(struct ph_unknown));
	return ret;
}

int ph_unknown_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_unknown_init(void* handler, struct configuration_t* c)
{
	struct ph_unknown* unknown = (struct ph_unknown*)handler;
	unknown->config = c;
	return 0;
}

int ph_unknown_deinit(void* handler)
{
	return 0;
}

int ph_unknown_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, ssize_t* len)
{
	struct unknown_struct* data = (struct unknown_struct*) malloc(sizeof(struct unknown_struct));


	if (*len > 0) 
	{
		char timestamp[100];
		create_timestamp(timestamp);
		snprintf(data->binaryLocation,MAX_PATH_LENGTH,"unknown/received/%s",timestamp);
		save_binarydata_to_file(data->binaryLocation,payload,*len);
	}        
	logger_get()->log_struct(logger_get(), conn, "server", data);

	return 0;
}

int ph_unknown_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
        msg(MSG_DEBUG,"unknown cts");
	struct unknown_struct* data = (struct unknown_struct*) malloc(sizeof(struct unknown_struct));


	if (*len > 0) {
		char timestamp[100];
		create_timestamp(timestamp);
		snprintf(data->binaryLocation,MAX_PATH_LENGTH,"unknown/sent/%s",timestamp);
		save_binarydata_to_file(data->binaryLocation,payload,*len);
	}

        logger_get()->log_struct(logger_get(), conn, "client", data);

        return 0;


}

int ph_unknown_handle_packet(void* handler, const char* packet, ssize_t len)
{
	return 0;
}

int ph_unknown_determine_target(void* handler, struct sockaddr_in* addr)
{
	msg(MSG_DEBUG,"determine target in udp");
	/*If necessary any time in the future to change the destination on-the-fly (like redirecting packets to 135/139/445 to special windows machines... etc)
	 *
	 * struct ph_unknown* unknown = (struct ph_unknown*)handler;
	if (conf_get_mode(unknown->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(unknown->config, "http", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
	}*/
	return 0;
}

