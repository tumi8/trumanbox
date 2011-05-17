#include "semaphore.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <common/msg.h>

void semaph_init() {
   int id;

   union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
   } argument;

   argument.val = 1;

   id = semget(KEY, 1, 0666 | IPC_CREAT);

   if(id < 0)
   {
      msg(MSG_FATAL, "Unable to obtain semaphore: %s", strerror(errno));
      exit(0);
   }

   if( semctl(id, 0, SETVAL, argument) < 0)
   {
      msg(MSG_FATAL, "Cannot set semaphore value: %s", strerror(errno));
      exit(0);
   }
}

void semaph_free() {
   int id;
   struct sembuf operations[1];

   int retval;

   id = semget(KEY, 1, 0666);
   if(id < 0)
   {
      msg(MSG_FATAL, "Program sema cannot find semaphore, exiting: %s", strerror(errno));
      exit(0);
   }

    // for increasing sem value
    operations[0].sem_num = 0;
    operations[0].sem_op = 1;
    operations[0].sem_flg = 0;

    retval = semop(id, operations, 1);

    if(retval != 0)
    {
	msg(MSG_FATAL, "sema: V-operation did not succeed: %s", strerror(errno));
	exit(1);
    }
}


void semaph_alloc() {
   int id;
   struct sembuf operations[1];

   int retval;

   id = semget(KEY, 1, 0666);
   if(id < 0)
   {
      msg(MSG_FATAL, "Program semb cannot find semaphore, exiting: %s", strerror(errno));
      exit(1);
   }

    // for decreasing sem value
    operations[0].sem_num = 0;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;

    retval = semop(id, operations, 1);

    if(retval != 0)
    {
	msg(MSG_FATAL, "semb: P-operation did not succeed: %s", strerror(errno));
	exit(1);
    }
}
