#include "unknown_udp.h"
#include "wrapper.h"
#include "helper_file.h"
#include "msg.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

struct ph_unknown_udp {
	struct configuration_t* config;
};

void* ph_unknown_udp_create()
{
	msg(MSG_DEBUG,"unknown udp create");
	void* ret = malloc(sizeof(struct ph_unknown_udp));
	return ret;
}

int ph_unknown_udp_destroy(void* handler)
{
	msg(MSG_DEBUG,"try to destroy...");free(handler);
	return 0;
}

int ph_unknown_udp_init(void* handler, struct configuration_t* c)
{
	msg(MSG_DEBUG,"unknown udp init");
	struct ph_unknown_udp* unknown_udp = (struct ph_unknown_udp*)handler;
	unknown_udp->config = c;

	return 0;
}

int ph_unknown_udp_deinit(void* handler)
{
	return 0;
}

int ph_unknown_udp_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, ssize_t* len)
{
	msg(MSG_DEBUG,"unknown udp stc");
	return 0;
}

int ph_unknown_udp_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{


	msg(MSG_DEBUG,"unknown udp cts");
	msg(MSG_DEBUG,"Handling udp unknown Len (%d)",*len);
	char location[1000];
	
	if (*len > 0) {
		char timestamp[1000];
		create_timestamp(timestamp);
		snprintf(location,1000,"unknown_udp/%s",timestamp);
		save_binarydata_to_file(location,payload,*len);
	}

        //logger_get()->log_struct(logger_get(), conn, "client", data);

        return 1;

}

int ph_unknown_udp_handle_packet(void* handler, const char* packet, ssize_t len)
{
	return 0;
}

int ph_unknown_udp_determine_target(void* handler, struct sockaddr_in* addr)
{
	msg(MSG_DEBUG,"determine target in udp");
	/*If necessary any time in the future to change the destination on-the-fly (like redirecting packets to 135/139/445 to special windows machines... etc)
	 *
	 * struct ph_unknown_udp* unknown_udp = (struct ph_unknown_udp*)handler;
	if (conf_get_mode(unknown_udp->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(unknown_udp->config, "http", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
	}*/
	return 0;
}

