#ifndef _SSL_HANDLER_H_
#define _SSL_HANDLER_H_

#include <common/definitions.h>
#include "tcp_handler.h"

class SSLHandler 
{
	public:
		SSLHandler(TcpHandler* tcpHandler);
		void run();
		
		uint16_t getSSLPort() const { return this->sslServerPort; }
	private:
		void log_to_db(char* filename, char* from);


		TcpHandler* tcphandler; 
		int serverSocket;
		int serverConnectionSocket;
		int clientSocket;
		char dest[IPLENGTH];
		u_int16_t destPort;
		u_int16_t sslServerPort;
};

#endif
