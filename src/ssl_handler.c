#include "ssl_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "msg.h"
#include "configuration.h"
#include "wrapper.h"
#include "tcp_handler.h"
#include "protocols/proto_handler.h"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

static char *pass;

/*The password code is not thread safe*/
static int password_cb(char *buf,int num,
  int rwflag,void *userdata)
  {
    if(num<strlen(pass)+1)
      return(0);

    strcpy(buf,pass);
    return(strlen(pass));
  }

SSL_CTX *initialize_ctx(keyfile,password)
  char *keyfile;
  char *password;
  {
    SSL_METHOD *meth;
    SSL_CTX *ctx;
    
      msg(MSG_DEBUG,"Global ssl system initialization");
      SSL_library_init();
      SSL_load_error_strings();
      
    
	msg(MSG_DEBUG,"creating ssl context");
    meth=SSLv23_method();
    ctx=SSL_CTX_new(meth);

    msg(MSG_DEBUG,"Load our keys and certificates");
	if(!(SSL_CTX_use_certificate_chain_file(ctx,     keyfile))) {
		msg(MSG_FATAL,"Can't read certificate file");
		return NULL;
	}
   pass=password;
    
    SSL_CTX_set_default_passwd_cb(ctx,password_cb);
    if(!(SSL_CTX_use_PrivateKey_file(ctx, keyfile,SSL_FILETYPE_PEM))) {
	msg(MSG_FATAL,"Can't read key file");
	return NULL;
    }
    /* Load the CAs we trust*/
    if(!(SSL_CTX_load_verify_locations(ctx,"root.pem",0)))
     {  msg(MSG_FATAL,"Can't read CA list");
	return NULL;
    }

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    SSL_CTX_set_verify_depth(ctx,1);
#endif
    
    return ctx;
  }
     
void destroy_ctx(ctx)
  SSL_CTX *ctx;
  {
    SSL_CTX_free(ctx);
  }


void load_dh_params(ctx,file)
  SSL_CTX *ctx;
  char *file;
  {
    DH *ret=0;
    BIO *bio;

    if ((bio=BIO_new_file(file,"r")) == NULL)
      {
      	msg(MSG_FATAL,"Couldn't open DH file");
	return;
	}

    ret=PEM_read_bio_DHparams(bio,NULL,NULL, NULL);
    BIO_free(bio);
    if(SSL_CTX_set_tmp_dh(ctx,ret)<0)
      msg(MSG_FATAL,"Couldn't set DH parameters");
  }


struct ssl_handler_t* sslhandler_create(struct tcp_handler_t* tcph)
{
	struct ssl_handler_t* ret = (struct ssl_handler_t*)malloc(sizeof(struct ssl_handler_t));
	ret->tcphandler = tcph;
	ret->inConnFd = 0; 
	ret->targetServiceFd = 0;
	ret->sslServerPort = 0; // we set up the ssl server port 
	ret->destPort = tcph->connection->dport;
	memcpy(ret->dest,tcph->connection->dest,IPLENGTH);
	struct sockaddr_in sin;
        int val=1;
	    
	if((ret->sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		msg(MSG_FATAL,"Couldn't make socket");
		return NULL;
	}
			
	memset(&sin,0,sizeof(sin));
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(0); // let the kernel choose a port
	setsockopt(ret->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
					
	if(bind(ret->sock,(struct sockaddr *)&sin,sizeof(sin))<0)
	{
		msg(MSG_FATAL,"Couldn't bind");
		return NULL;
	}

	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(server_addr);
	int rc = getsockname(ret->sock,(struct sockaddr *)&server_addr, &addrlen);
	rc++;
	ret->sslServerPort = ntohs(server_addr.sin_port);
	listen(ret->sock,20);  
	return ret;
}


void sslhandler_destroy(struct ssl_handler_t* t)
{
	free(t);
}


void sslhandler_run(struct ssl_handler_t* sslh)
{
	int s_server_session_id_context = 1;
	BIO *sbio;
	SSL_CTX *ctx;
	SSL *ssl;
	int serverSocket,r;

	msg(MSG_DEBUG,"Build our SSL context");
	ctx=initialize_ctx("sslserver.pem","password");
	msg(MSG_DEBUG,"initialized ctx");
	load_dh_params(ctx,"dh1024.pem");

	SSL_CTX_set_session_id_context(ctx,(void*)&s_server_session_id_context,sizeof s_server_session_id_context); 

	while(1) {
		msg(MSG_DEBUG,"entered endless loop..");
	      if((serverSocket=accept(sslh->sock,0,0))<0)
	           {
		   	msg(MSG_FATAL,"problem accepting");
			continue;
		   }

		
	   	sbio=BIO_new_socket(serverSocket,BIO_NOCLOSE);
		ssl=SSL_new(ctx);
		SSL_set_bio(ssl,sbio,sbio);
					           
		if((r=SSL_accept(ssl)<=0))
		{
			msg(MSG_FATAL,"SSL accept error");
			continue;
		}
				

		//TODO: SSL Client initialization
	
		//TODO: select() the fds connected 
		
		//TODO: If Server FD -> write to real target fd (client fd)

		//TODO: If Client FD -> write to trumanbox (server fd)
	
	


	}
	/*int maxfd;
	struct sockaddr_in targetServAddr;
	ssize_t r;
	fd_set rset;
	struct timeval tv;
		

	// TODO: SSL Server initialization
	
	// TODO: SSL Client initializiation

	
	
	FD_ZERO(&rset);
	//FD_SET(sslh->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
	FD_SET(sslh->inConnFd, &rset);
	//maxfd = max(sslh->targetServiceFd, sslh->inConnFd) + 1;
	maxfd = sslh->inConnFd + 1;

	// wait 3 seconds for initial client payload
	// try to receive server payload if there is no 
	// payload from the client.
	tv.tv_sec = 5;
	tv.tv_usec = 0; 
	
	while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(sslh->targetServiceFd, &rset)) {
			// we received data from the internet server

			msg(MSG_DEBUG, "Received data from target server!");
			r = read(sslh->targetServiceFd, payloadRead, MAXLINE - 1);
			
			// TODO: Read payload and log to file

			// TODO: write payload to the client socket

			if (!r) {
				msg(MSG_DEBUG, "Target closed the connection...");
				goto out;
			}
			
		} else if (FD_ISSET(sslh->inConnFd, &rset)) {
			msg(MSG_DEBUG, "Received data from infected machine!");
			r = read(sslh->inConnFd, payloadRead, MAXLINE - 1);
			if (!r) {
				msg(MSG_DEBUG, "Infected machine closed the connection...");
				goto out;
			}

			// TODO: read payload and log to file
			//
			// TODO: write payload to the server socket
		} else {

		}
		FD_ZERO(&rset);
		FD_SET(sslh->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
		FD_SET(sslh->inConnFd, &rset);
		maxfd = max(sslh->targetServiceFd, sslh->inConnFd) + 1;
		tv.tv_sec = 300;
		tv.tv_usec = 0; 
	}

out:
	Close_conn(sslh->inConnFd, "incoming connection, because we are done with this connection");
	Close_conn(sslh->targetServiceFd, "connection to targetservice, because we are done with this connection");
*/
}

