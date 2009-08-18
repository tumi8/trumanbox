#include "wrapper.h"
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "msg.h"

// in the following some wrapper functions
void Write(int fd, void *ptr, size_t nbytes) {
	if (write(fd, ptr, nbytes) != nbytes)
		msg(MSG_ERROR, "write error: %s", strerror(errno));
}

int Socket(int family, int type, int protocol) {
	int n;
	
	if ( (n = socket(family, type, protocol)) < 0)
		msg(MSG_ERROR, "socket error: %s", strerror(errno));
	return (n);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			msg(MSG_ERROR, "accept error: %s", strerror(errno));
	}
	return(n);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (bind(fd, sa, salen) < 0)
		perror("bind error");
}

int Connect(int fd, const struct sockaddr *sa, socklen_t salen) {
	int status;

	if ((status = connect(fd, sa, salen)) < 0)
		perror("connect error");

	return status;
}

void Listen(int fd, int backlog) {
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		perror("listen error");
}

pid_t Fork(void) {
	pid_t	pid;

	if ( (pid = fork()) == -1)
		perror("fork error");
	return(pid);
}

void Pipe(int *ptr_pipe) {
	if ( pipe(ptr_pipe) < 0)
		perror("could not create pipe");
}

ssize_t Read(int fd, char *read_buf, size_t count) {
	int	read_cnt = 0;

	if ( (read_cnt = read(fd, read_buf, count)) < 0)
		perror("read error");
	
	return(read_cnt);
}

void Close(int fd) {
	msg(MSG_DEBUG, "(pid: %d) closing file", getpid());
	if (close(fd) == -1)
		msg(MSG_ERROR, "close error: %s", strerror(errno));
}

void Close_file(FILE *fd) {
	msg(MSG_DEBUG, "(pid: %d) closing File", getpid());
	if (fclose(fd) == -1)
		msg(MSG_ERROR, "close error: %s", strerror(errno));
}

void Close_conn(int fd, const char *mark) {
	msg(MSG_DEBUG, "(pid: %d) closing connection: %s", getpid(), mark);
	//sleep(1);
	if (shutdown(fd, SHUT_RDWR) == -1)
		msg(MSG_ERROR, "close connection error: %s", strerror(errno));
}
// FIXME: error handling
void Inet_pton(int family, const char *strptr, void *addrptr) {
	int		n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		msg(MSG_ERROR, "inet_pton error for %s: %s", strptr, strerror(errno));	/* errno set */
	else if (n == 0)
		msg(MSG_ERROR, "inet_pton error for %s", strptr);	/* errno not set */

	/* nothing to return */
}

// FIXME: error handling
void Inet_ntop(int af, const void *src, char *dst, socklen_t cnt) {
	
	if ( NULL == (inet_ntop(af, src, dst, cnt) ) ) {
		msg(MSG_ERROR, "inet_ntop error: %s", strerror(errno));
	}
}

ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr) {
        ssize_t         n;

        if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
                msg(MSG_ERROR, "recvfrom error: %s", strerror(errno));
        return(n);
}

void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen) {
        if (sendto(fd, ptr, nbytes, flags, sa, salen) != nbytes)
                msg(MSG_ERROR, "sendto error: %s", strerror(errno));
}

void Exit(int status) {
	msg(MSG_DEBUG, "process %d will exit now with status %d", getpid(), status);
	exit(status);
}
