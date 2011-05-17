#include "unknown.h"
#include "wrapper.h"
#include "helper_file.h"
#include <stdlib.h>
#include <string.h>

#include <common/msg.h>
#include <logging/logbase.h>

UnknownHandler::UnknownHandler(const Configuration& config)
	: ProtoHandler(config)
{

}

int UnknownHandler::payloadServerToClient(connection_t* conn,  const char* payload, ssize_t* len)
{
	struct unknown_struct* data = (struct unknown_struct*) malloc(sizeof(struct unknown_struct));


	if (*len > 0) 
	{
		char timestamp[100];
		create_timestamp(timestamp);
		snprintf(data->binaryLocation,MAX_PATH_LENGTH,"unknown/received/%s",timestamp);
		save_binarydata_to_file(data->binaryLocation,payload,*len);
	}        
	logger_get()->logStruct(conn, "server", data);

	return 0;
}

int UnknownHandler::payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len)
{
        msg(MSG_DEBUG,"unknown cts");
	struct unknown_struct* data = (struct unknown_struct*) malloc(sizeof(struct unknown_struct));


	if (*len > 0) {
		char timestamp[100];
		create_timestamp(timestamp);
		snprintf(data->binaryLocation,MAX_PATH_LENGTH,"unknown/sent/%s",timestamp);
		save_binarydata_to_file(data->binaryLocation,payload,*len);
	}

        logger_get()->logStruct(conn, "client", data);

        return 0;


}

int UnknownHandler::determineTarget(struct sockaddr_in* addr)
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

