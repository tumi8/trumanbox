#include "ssl_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
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
static int client_auth=0;
static int s_server_auth_session_id_context = 2;
#define CLIENT_AUTH_REQUEST 1
#define CLIENT_AUTH_REQUIRE 2
#define CLIENT_AUTH_REHANDSHAKE 3

static int tcp_connect(char* host, int port)
  
  {
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;
    
    if(!(hp=gethostbyname(host)))
      {
      msg(MSG_DEBUG,"Couldn't resolve host");
      return -1;
      }
    memset(&addr,0,sizeof(addr));
    addr.sin_addr=*(struct in_addr*)
      hp->h_addr_list[0];
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);

    if((sock=socket(AF_INET,SOCK_STREAM,
      IPPROTO_TCP))<0)
       {
       msg(MSG_DEBUG,"Couldn't create socket");
	return -1;
	}
    if(connect(sock,(struct sockaddr *)&addr,
      sizeof(addr))<0)
      {
      msg(MSG_DEBUG,"Couldn't connect socket");
      return -1;
      }
    
    return sock;
  }



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
	ret->serverSocket = 0; 
	ret->serverConnectionSocket = 0;
	ret->clientSocket = 0;
	ret->sslServerPort = 0; // we set up the ssl server port 
	ret->destPort = tcph->connection->dport;
	memcpy(ret->dest,tcph->connection->dest,IPLENGTH);
	struct sockaddr_in sin;
        int val=1;
	    
	if((ret->serverSocket=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		msg(MSG_FATAL,"Couldn't make socket");
		return NULL;
	}
			
	memset(&sin,0,sizeof(sin));
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(0); // let the kernel choose a port
	setsockopt(ret->serverSocket,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
					
	if(bind(ret->serverSocket,(struct sockaddr *)&sin,sizeof(sin))<0)
	{
		msg(MSG_FATAL,"Couldn't bind");
		return NULL;
	}

	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(server_addr);
	int rc = getsockname(ret->serverSocket,(struct sockaddr *)&server_addr, &addrlen);
	rc++;
	ret->sslServerPort = ntohs(server_addr.sin_port);
	listen(ret->serverSocket,20);  
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
	BIO *cbio;
	SSL_CTX *ctx;
	SSL *ssl;
	SSL *sslClient;
	int r;

	msg(MSG_DEBUG,"Build our SSL context");
	ctx=initialize_ctx("sslserver.pem","password");
	msg(MSG_DEBUG,"initialized ctx");
	load_dh_params(ctx,"dh1024.pem");

	SSL_CTX_set_session_id_context(ctx,(void*)&s_server_session_id_context,sizeof s_server_session_id_context); 

	while(1) {
		msg(MSG_DEBUG,"entered endless loop..");
	      if((sslh->serverConnectionSocket=accept(sslh->serverSocket,0,0))<0)
	           {
		   	msg(MSG_FATAL,"problem accepting");
			continue;
		   }

		
		msg(MSG_DEBUG,"initialized BIOs");
	   	sbio=BIO_new_socket(sslh->serverConnectionSocket,BIO_NOCLOSE);
		ssl=SSL_new(ctx);
		SSL_set_bio(ssl,sbio,sbio);
		
		msg(MSG_DEBUG,"ready for SSL accept");
		if((r=SSL_accept(ssl)<=0))
		{
			msg(MSG_FATAL,"SSL accept error");
			goto shutdown;
		}

    	sslh->clientSocket=tcp_connect(sslh->dest,sslh->destPort);
	if (sslh->clientSocket == -1) goto shutdown;
    
    	/* Connect the Client SSL socket */
    	sslClient=SSL_new(ctx);
    	cbio=BIO_new_socket(sslh->clientSocket,BIO_NOCLOSE);
    	SSL_set_bio(sslClient,cbio,cbio);

    	if(SSL_connect(sslClient)<=0)
     	{
		msg(MSG_FATAL,"SSL connect error");
		goto shutdown;
	}
	
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	int maxfd;
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(sslh->serverConnectionSocket, &rset);
	maxfd = sslh->serverConnectionSocket + 1;
		char buf[2*MAXLINE];
		int len;
		BIO *io,*ssl_bio;

		io=BIO_new(BIO_f_buffer());
		ssl_bio=BIO_new(BIO_f_ssl());
		BIO_set_ssl(ssl_bio,ssl,BIO_CLOSE);
		BIO_push(io,ssl_bio);


		while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
			if (FD_ISSET(sslh->serverConnectionSocket, &rset)) {
				msg(MSG_DEBUG,"received something from the malware client");
				r=BIO_gets(io,buf,2*MAXLINE-1);

				switch(SSL_get_error(ssl,r)){
				case SSL_ERROR_NONE:
					len=r;
					break;
				case SSL_ERROR_ZERO_RETURN:
					goto shutdown;
					break;
				default:
					msg(MSG_FATAL,"SSL read problem");
				}

			msg(MSG_DEBUG,"we rcvd %d bytes",r);
			
			int bytesWritten= 0;
			while (bytesWritten != r) {
				msg(MSG_DEBUG,"try to write '%s'",buf+bytesWritten);
				bytesWritten = bytesWritten + SSL_write(sslClient,buf+bytesWritten,r-bytesWritten);
				msg(MSG_DEBUG,"we have written %d bytes  to %s:%d",bytesWritten,sslh->dest,sslh->destPort);
				switch(SSL_get_error(ssl,r)){      
					case SSL_ERROR_NONE:
						if(bytesWritten!=r){
							msg(MSG_FATAL,"Incomplete write!");
						} 
						break;
					default:
						{
						msg(MSG_FATAL,"SSL write problem");
						goto shutdown;
						}
				}
			}
			/* Look for the blank line that signals
			 the end of the HTTP headers */
			if(!strcmp(buf,"\r\n") || strcmp(buf,"\n"))
				break;
			
			/* Now perform renegotiation if requested */
			if(client_auth==CLIENT_AUTH_REHANDSHAKE){
			SSL_set_verify(ssl,SSL_VERIFY_PEER |
			SSL_VERIFY_FAIL_IF_NO_PEER_CERT,0);

			/* Stop the client from just resuming the
			 un-authenticated session */
			SSL_set_session_id_context(ssl,
			(void *)&s_server_auth_session_id_context,
			sizeof(s_server_auth_session_id_context));

			if(SSL_renegotiate(ssl)<=0)
			msg(MSG_FATAL,"SSL renegotiation error");
			if(SSL_do_handshake(ssl)<=0)
			msg(MSG_FATAL,"SSL renegotiation error");
			ssl->state=SSL_ST_ACCEPT;
			if(SSL_do_handshake(ssl)<=0)
			msg(MSG_FATAL,"SSL renegotiation error");
			}

			if((r=BIO_puts
			(io,"HTTP/1.0 200 OK\r\n"))<=0)
			msg(MSG_FATAL,"Write error");
			if((r=BIO_puts
			(io,"Server: EKRServer\r\n\r\n"))<=0)
			msg(MSG_FATAL,"Write error");
			if((r=BIO_puts
			(io,"Server test page\r\n"))<=0)
			msg(MSG_FATAL,"Write error");

			if((r=BIO_flush(io))<0)
			msg(MSG_FATAL,"Error flushing BIO");


		}
		else if (FD_ISSET(sslh->clientSocket, &rset)) {

			msg(MSG_DEBUG,"omg we received something from the ssl real server!");
		}
		else {
                        // We received a timeout. There are now two possiblities:
                        // 1.) We already identified the protocol: There is something wrong, as there should not be any timeout
                        // 2.) We did not identify the payload: We need to perform some  actions to enable payload identification
                        msg(MSG_DEBUG,"Received a timeout???");
                }
                FD_ZERO(&rset);
                FD_SET(sslh->clientSocket, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
                FD_SET(sslh->serverConnectionSocket, &rset);
                maxfd = max(sslh->serverConnectionSocket, sslh->clientSocket) + 1;
                tv.tv_sec = 300;
                tv.tv_usec = 0;

	}
		shutdown:
		r=SSL_shutdown(ssl);
		if(!r){
		/* If we called SSL_shutdown() first then
		 we always get return value of '0'. In
		 this case, try again, but first send a
		 TCP FIN to trigger the other side's
		 close_notify*/
		shutdown(sslh->serverConnectionSocket,1);
		r=SSL_shutdown(ssl);
		}

		switch(r){  
		case 1:
		break; /* Success */
		case 0:
		case -1:
		default:
		msg(MSG_FATAL,"Shutdown failed");
		}

		SSL_free(ssl);
		close(sslh->serverConnectionSocket);
		SSL_CTX_free(ctx);
	       	close(sslh->clientSocket);
		exit(0);




				
	
		//TODO: select() the fds connected 
		
		//TODO: If Server FD -> write to real target fd (client fd)

		//TODO: If Client FD -> write to trumanbox (server fd)
	
	


	}
}

