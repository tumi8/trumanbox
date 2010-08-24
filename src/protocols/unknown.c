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
			if (payload[0] == '\x16') {
			msg(MSG_DEBUG,"Handshake");
			if (payload[1] == '\x3' && payload[2] == '\x0') msg(MSG_DEBUG,"SSL v3");
			else if (payload[1] == '\x3' && payload[2] == '\x1') msg(MSG_DEBUG,"TLS 1.0");
			else if (payload[1] == '\x3' && payload[2] == '\x2') msg(MSG_DEBUG,"TLS 1.1");
			else if (payload[1] == '\x3' && payload[2] == '\x3') msg(MSG_DEBUG,"TLS 1.2");
			char MessageType[100];
			switch (payload[5]) {
				case '\x0': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"HelloRequest"); break;
				case '\x1': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Client Hello"); break;
				case '\x2': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Server Hello"); break;
				case '\xb': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Certificate"); break;
				case '\xc': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ServerKeyExchange"); break;
				case '\xd': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"CertificateRequest"); break;
				case '\xe': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ServerHelloDone"); break;
				case '\xf': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Certificate Verify"); break;
				case '\xf0': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ClientKeyExchange"); break;
				case '\xf4': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Finished"); break; 
			}
			msg(MSG_DEBUG,"messagetype: %s",MessageType);

		}
		else if (payload[0] == '\x17')
			msg(MSG_DEBUG,"application");char timestamp[100];
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
		if (payload[0] == '\x16') {
			msg(MSG_DEBUG,"Handshake");
			if (payload[1] == '\x3' && payload[2] == '\x0') msg(MSG_DEBUG,"SSL v3");
			else if (payload[1] == '\x3' && payload[2] == '\x1') msg(MSG_DEBUG,"TLS 1.0");
			else if (payload[1] == '\x3' && payload[2] == '\x2') msg(MSG_DEBUG,"TLS 1.1");
			else if (payload[1] == '\x3' && payload[2] == '\x3') msg(MSG_DEBUG,"TLS 1.2");
			char MessageType[100];
			switch (payload[5]) {
				case '\x0': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"HelloRequest"); break;
				case '\x1': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Client Hello"); break;
				case '\x2': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Server Hello"); break;
				case '\xb': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Certificate"); break;
				case '\xc': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ServerKeyExchange"); break;
				case '\xd': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"CertificateRequest"); break;
				case '\xe': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ServerHelloDone"); break;
				case '\xf': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Certificate Verify"); break;
				case '\xf0': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"ClientKeyExchange"); break;
				case '\xf4': msg(MSG_DEBUG,"hallo"); strcpy(MessageType,"Finished"); break; 
			}
			msg(MSG_DEBUG,"messagetype: %s",MessageType);

		}
		else if (payload[0] == '\x17')
			msg(MSG_DEBUG,"application");
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

