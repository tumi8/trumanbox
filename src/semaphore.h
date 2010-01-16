#include <stdio.h>
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

#include <sys/ipc.h>
#include <sys/sem.h>

#define KEY (1492)

void semaph_init();
void semaph_free();
void semaph_alloc();
