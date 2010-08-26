#include "ssl.h"
#include "wrapper.h"
#include "helper_file.h"
#include "msg.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
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


static void print_stuff(BIO *bio, SSL *s, int full)
	
{
	X509 *peer=NULL;
	char *p;
	static const char *space="                ";
	char buf[BUFSIZ];
	STACK_OF(X509) *sk;
	STACK_OF(X509_NAME) *sk2;
	const SSL_CIPHER *c;
	X509_NAME *xn;
	int j,i;
#ifndef OPENSSL_NO_COMP
	const COMP_METHOD *comp, *expansion;
#endif

	if (full)
		{
		int got_a_chain = 0;

		sk=SSL_get_peer_cert_chain(s);
		if (sk != NULL)
			{
			got_a_chain = 1; /* we don't have it for SSL2 (yet) */

			BIO_printf(bio,"---\nCertificate chain\n");
			for (i=0; i<sk_X509_num(sk); i++)
				{
				X509_NAME_oneline(X509_get_subject_name(
					sk_X509_value(sk,i)),buf,sizeof buf);
				BIO_printf(bio,"%2d s:%s\n",i,buf);
				X509_NAME_oneline(X509_get_issuer_name(
					sk_X509_value(sk,i)),buf,sizeof buf);
				BIO_printf(bio,"   i:%s\n",buf);
				PEM_write_bio_X509(bio,sk_X509_value(sk,i));
	
			}

		BIO_printf(bio,"---\n");
		peer=SSL_get_peer_certificate(s);
		if (peer != NULL)
			{
			BIO_printf(bio,"Server certificate\n");
			if (!(got_a_chain)) /* Redundant if we showed the whole chain */
				PEM_write_bio_X509(bio,peer);
			X509_NAME_oneline(X509_get_subject_name(peer),
				buf,sizeof buf);
			BIO_printf(bio,"subject=%s\n",buf);
			X509_NAME_oneline(X509_get_issuer_name(peer),
				buf,sizeof buf);
			BIO_printf(bio,"issuer=%s\n",buf);
			}
		else
			BIO_printf(bio,"no peer certificate available\n");

		sk2=SSL_get_client_CA_list(s);
		if ((sk2 != NULL) && (sk_X509_NAME_num(sk2) > 0))
			{
			BIO_printf(bio,"---\nAcceptable client certificate CA names\n");
			for (i=0; i<sk_X509_NAME_num(sk2); i++)
				{
				xn=sk_X509_NAME_value(sk2,i);
				X509_NAME_oneline(xn,buf,sizeof(buf));
				BIO_write(bio,buf,strlen(buf));
				BIO_write(bio,"\n",1);
				}
			}
		else
			{
			BIO_printf(bio,"---\nNo client certificate CA names sent\n");
			}
		p=SSL_get_shared_ciphers(s,buf,sizeof buf);
		if (p != NULL)
			{
			/* This works only for SSL 2.  In later protocol
			 * versions, the client does not know what other
			 * ciphers (in addition to the one to be used
			 * in the current connection) the server supports. */

			BIO_printf(bio,"---\nCiphers common between both SSL endpoints:\n");
			j=i=0;
			while (*p)
				{
				if (*p == ':')
					{
					BIO_write(bio,space,15-j%25);
					i++;
					j=0;
					BIO_write(bio,((i%3)?" ":"\n"),1);
					}
				else
					{
					BIO_write(bio,p,1);
					j++;
					}
				p++;
				}
			BIO_write(bio,"\n",1);
			}

		BIO_printf(bio,"---\nSSL handshake has read %ld bytes and written %ld bytes\n",
			BIO_number_read(SSL_get_rbio(s)),
			BIO_number_written(SSL_get_wbio(s)));
		}
	BIO_printf(bio,((s->hit)?"---\nReused, ":"---\nNew, "));
	c=SSL_get_current_cipher(s);
	BIO_printf(bio,"%s, Cipher is %s\n",
		SSL_CIPHER_get_version(c),
		SSL_CIPHER_get_name(c));
	if (peer != NULL) {
		EVP_PKEY *pktmp;
		pktmp = X509_get_pubkey(peer);
		BIO_printf(bio,"Server public key is %d bit\n",
							 EVP_PKEY_bits(pktmp));
		EVP_PKEY_free(pktmp);
	}

#ifndef OPENSSL_NO_COMP
	comp=SSL_get_current_compression(s);
	expansion=SSL_get_current_expansion(s);
	BIO_printf(bio,"Compression: %s\n",
		comp ? SSL_COMP_get_name(comp) : "NONE");
	BIO_printf(bio,"Expansion: %s\n",
		expansion ? SSL_COMP_get_name(expansion) : "NONE");
#endif
	SSL_SESSION_print(bio,SSL_get_session(s));
	BIO_printf(bio,"---\n");
	if (peer != NULL)
		X509_free(peer);
	/* flush, or debugging output gets mixed with http response */
	(void)BIO_flush(bio);
	}
}

static int http_request(SSL* ssl, char* host, int port, char* filename)
  {
  char *REQUEST_TEMPLATE=
     "GET / HTTP/1.1\r\nUser-Agent: HTTPS-Client\r\nHost: %s:%d\r\n\r\n";
    char *request = 0;
    char buf[MAX_LINE_LENGTH];
    int r;
    int len, request_len;
    
    /* Now construct our HTTP request */
    request_len=strlen(REQUEST_TEMPLATE)+
      strlen(host)+6;
    if(!(request=(char *)malloc(request_len)))
      {
      msg(MSG_FATAL,"Couldn't allocate request");
      return -1;
      }
    snprintf(request,request_len,REQUEST_TEMPLATE,
      host,port);

    /* Find the exact request_len */
    request_len=strlen(request);

    r=SSL_write(ssl,request,request_len);
    switch(SSL_get_error(ssl,r)){      
      case SSL_ERROR_NONE:
        if(request_len!=r)
        {
      		msg(MSG_FATAL,"Incomplete write!");
      		return -1;
      	} 
        break;
        default:
         {
      		msg(MSG_FATAL,"SSL write problem");
      		return -1;
	 }
    }
    
    /* Now read the server's response, assuming
       that it's terminated by a close */
    while(1){
      r=SSL_read(ssl,buf,MAX_LINE_LENGTH);
      switch(SSL_get_error(ssl,r)){
        case SSL_ERROR_NONE:
          len=r;
          break;
        case SSL_ERROR_ZERO_RETURN:
          goto shutdown;
        case SSL_ERROR_SYSCALL:
          msg(MSG_FATAL,"SSL Error: Premature close\n");
          goto done;
        default:
          {
      		msg(MSG_FATAL,"SSL read problem");
      		return -1;
	 }         
	}

     // if  (fwrite(buf,1,len,stdout));
    if (append_binarydata_to_file(filename,buf,len));
    }
    
  shutdown:
    r=SSL_shutdown(ssl);
    switch(r){
      case 1:
        break; /* Success */
      case 0:
      case -1:
      default:
	msg(MSG_FATAL,"Shutdown failed");
    }
    
  done:
    SSL_free(ssl);
    free(request);
    return 0;
  }


static void get_server_ssl_information(connection_t* conn, char* filename, char* filenamerequest) {

	SSL_CTX *ctx;
	SSL *ssl;
	BIO *sbio;
	int sock;
	/* Build our SSL context*/

    	SSL_METHOD *meth;
    
		/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();
	

    	/* Set up a SIGPIPE handler */
	//signal(SIGPIPE,sigpipe_handle);
			    
	/* Create our context*/
	meth=SSLv23_method();
	ctx=SSL_CTX_new(meth);
    	sock=tcp_connect(conn->dest,conn->dport);
	if (sock == -1) return;
    /* Connect the SSL socket */
    ssl=SSL_new(ctx);
    sbio=BIO_new_socket(sock,BIO_NOCLOSE);
    SSL_set_bio(ssl,sbio,sbio);

    if(SSL_connect(ssl)<=0)
     	{
	msg(MSG_FATAL,"SSL connect error");
	return;
	}
	
	BIO* biofile = BIO_new_file(filename,"w");
 	print_stuff(biofile,ssl,1);

    /* Now make our HTTP request*/ 
    	if (conn->dport == 443) 
    	{
	// We have 443 as destination port and therefore the protocol is probably HTTPS, we send a dummy request 'GET /' to obtain some server information
		http_request(ssl,conn->dest,conn->dport,filenamerequest);
	}
	else 
	{
		strcpy(filenamerequest,"");	
	}
    /* Shutdown the socket */
   SSL_CTX_free(ctx);
    close(sock);

}

struct ph_ssl {
	struct configuration_t* config;
};

void* ph_ssl_create()
{
	void* ret = malloc(sizeof(struct ph_ssl));
	return ret;
}

int ph_ssl_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_ssl_init(void* handler, struct configuration_t* c)
{
	struct ph_ssl* ssl = (struct ph_ssl*)handler;
	ssl->config = c;
	return 0;
}

int ph_ssl_deinit(void* handler)
{
	return 0;
}

int ph_ssl_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, ssize_t* len)
{
        

	msg(MSG_DEBUG,"received SSL server msg");
	return 0;
}

int ph_ssl_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
  //              logger_get()->log_struct(logger_get(), conn, "client", data);
	if (conn->log_client_struct_initialized == 0) {
		struct ssl_struct* logdata = (struct ssl_struct*) malloc(sizeof(struct ssl_struct));
		
		snprintf(logdata->server_cert,MAX_PATH_LENGTH,"ssl/Cert_Info_%s",conn->timestamp);
		snprintf(logdata->http_request,MAX_PATH_LENGTH,"ssl/Request_%s",conn->timestamp);
		strcpy(logdata->sslVersion,conn->sslVersion);
		get_server_ssl_information(conn,logdata->server_cert,logdata->http_request);
		conn->log_client_struct_initialized = 1;
		return logger_get()->log_struct(logger_get(), conn, "client", logdata);


	}
	msg(MSG_DEBUG,"received SSL client msg");
        return 0;


}

int ph_ssl_handle_packet(void* handler, const char* packet, ssize_t len)
{
	return 0;
}

int ph_ssl_determine_target(void* handler, struct sockaddr_in* addr)
{
	// if necessary in the future, redirect to local running SSL Services (HTTPS, FTPS, SMTPS ....)
	return 0;
}

