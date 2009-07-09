#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

void Write(int fd, void *ptr, size_t nbytes);
int Socket(int family, int type, int protocol);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
void Listen(int fd, int backlog);
pid_t Fork(void);
void Pipe(int *ptr_pipe);
ssize_t Read(int fd, char *read_buf, size_t count);
void Close(int fd);
void Close_file(FILE *fd);
void Close_conn(int fd, const char *mark);
void Inet_pton(int family, const char *strptr, void *addrptr);
void Inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr);
void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen);
void Exit(int status);
