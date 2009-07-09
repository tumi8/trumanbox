#include "semaphore.h"

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
      fprintf(stderr, "Unable to obtain semaphore.\n");
      exit(0);
   }

   if( semctl(id, 0, SETVAL, argument) < 0)
   {
      fprintf( stderr, "Cannot set semaphore value.\n");
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
      fprintf(stderr, "Program sema cannot find semaphore, exiting.\n");
      exit(0);
   }

    // for increasing sem value
    operations[0].sem_num = 0;
    operations[0].sem_op = 1;
    operations[0].sem_flg = 0;

    retval = semop(id, operations, 1);

    if(retval != 0)
    {
	printf("sema: V-operation did not succeed.\n");
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
      fprintf(stderr, "Program semb cannot find semaphore, exiting.\n");
      exit(1);
   }

    // for decreasing sem value
    operations[0].sem_num = 0;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;

    retval = semop(id, operations, 1);

    if(retval != 0)
    {
	printf("semb: P-operation did not succeed.\n");
	exit(1);
    }
}
