

	


























































































#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <assert.h>

typedef struct simplelock lock;

typedef struct struct_ANL_Globals {
   int nextPid;	
   
	lock *anlLock;
;
} ANLGlobalsStruct;

ANLGlobalsStruct *ANLGlobals;

typedef struct anl_barrier {
    
	lock *lock;

    volatile int count;
    volatile int spin;
    volatile int inuse;
} anl_barrier;

typedef struct ticket_lock {
    
	lock *lock;

    volatile int next_available;
    volatile int currently_serving;
} ticket_lock;

typedef struct queue_lock {
    
	lock *lock;

    volatile int next_available;
    volatile int next_on;
    volatile int *array;
} queue_lock;

int _anl_reserved_fd;
int _anl_reserved_pid;
int _anl_reserved_nprocs;


typedef struct {
  unsigned int *start;
  unsigned int *end;
  int *a;
  int N,k,M;
  int numprocs;
  
	queue_lock *lock;

} GM;

GM *gm;

void benchmark(void) {
  register int i, j, p, q;
    int pid,N,k,M;
    N = gm->N;
    k = gm->k;
    M = gm->M;
    
{
  s_lock(ANLGlobals->anlLock);;
  pid = ANLGlobals->nextPid++;
  s_unlock(ANLGlobals->anlLock);;
};
    //M = pid*(gm->M);
	p = 0;
	q = 0;
    
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   gm->start[pid] = ((tv.tv_sec & 0x7ff) * 1000000) + tv.tv_usec;
}

	for (i=0; i <= N-1; i++) {
	    {
   int my_place;
   int index;
   s_lock(gm->lock->lock);;
   my_place = gm->lock->next_available++;
   if (gm->lock->next_available == gm->numprocs)
	  gm->lock->next_available = 0;
   s_unlock(gm->lock->lock);;
   index = my_place*(256/sizeof(int));
   while(gm->lock->array[index] == 0)
       ;
   gm->lock->array[index] = 0;
   index = ((my_place+1) % gm->numprocs)*(256/sizeof(int));
   gm->lock->next_on = index;
}
		for (j=0; j <= k-1; j++) { 
		  q++;
		}
		gm->lock->array[gm->lock->next_on] = 1;
		for (j=0; j <= M-1; j++) {
		  p++;
		}
	}
	
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   gm->end[pid] = ((tv.tv_sec & 0x7ff) * 1000000) + tv.tv_usec;
}
	gm->a[pid] = p+q;
}

int main(int argc, char **argv) {
  int P, i;
  unsigned int t1, t2;
  unsigned int *start;
  unsigned int *end;
  int *a;
	
{
   if ((_anl_reserved_fd=open("/dev/zero", O_RDWR)) == -1) {
      perror("ANL Init cannot open /dev/zero!");
      exit(99);
   }

   if ((ANLGlobals = (ANLGlobalsStruct *)mmap(NULL, 
	sizeof(ANLGlobalsStruct), PROT_READ|PROT_WRITE, MAP_SHARED, 
	_anl_reserved_fd, 0)) == MAP_FAILED) {
     perror ("ANL Global Initialization failed!");
     exit(99);
   }

   
   if ((ANLGlobals->anlLock = (lock*)mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, 
       	MAP_SHARED, _anl_reserved_fd, 0)) == MAP_FAILED) {
      perror ("LockInit failed!");
   }
   s_lock_init (ANLGlobals->anlLock);
;
   _anl_reserved_nprocs = 1;
}
	if (argc!=4) {
		printf("Usage: my_lock P k M\nAborting...\n");
		exit(0);
	}

	// Initialize global memory
	gm = (GM *)
   mmap(NULL, sizeof(GM), PROT_READ|PROT_WRITE, MAP_SHARED, _anl_reserved_fd, 0);
;
	P = gm->numprocs = atoi(argv[1]);
	assert(P > 0);
	assert(P <= 8);
	
   
   if ((gm->lock = (queue_lock *)mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, 
       	MAP_SHARED, _anl_reserved_fd, 0)) == MAP_FAILED) {
      perror ("Queue Lock Init failed!");
   }

    
   if ((gm->lock->lock = (lock*)mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, 
       	MAP_SHARED, _anl_reserved_fd, 0)) == MAP_FAILED) {
      perror ("LockInit failed!");
   }
   s_lock_init (gm->lock->lock);
;
	gm->lock->array = (int*)
   mmap(NULL, gm->numprocs*256, PROT_READ|PROT_WRITE, MAP_SHARED, _anl_reserved_fd, 0);
;
	int iter;
	for (iter=0; iter < gm->numprocs; iter++) {
		gm->lock->array[iter*(256/sizeof(int))] = 0;
	}
    gm->lock->array[0] = 1;
    gm->lock->next_available = 0;
    gm->lock->next_on = 0;


	// Initialize arrays in global memory
	start = gm->start = (unsigned int*)
   mmap(NULL, P*sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED, _anl_reserved_fd, 0);
;
	end = gm->end = (unsigned int*)
   mmap(NULL, P*sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED, _anl_reserved_fd, 0);
;
	a = gm->a = (int*)
   mmap(NULL, P*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, _anl_reserved_fd, 0);
;

	// Initialize design parameters
	gm->N = 2000000;
	gm->k = atoi(argv[2]);
	gm->M = atoi(argv[3]);

	// Initialize processes per processor
	for(i = 0; i < P-1; i++) {
		
{
   _anl_reserved_nprocs++;
   if ((_anl_reserved_pid = fork()) == -1) {
     perror ("Create failed!");
     exit (99);
   }

   if (!_anl_reserved_pid) {
      // child
      benchmark();
      exit(0);
   }
}
	}

	// Run the processes
	benchmark();
	
{
  int _anl_reserved_temp; 
  for (_anl_reserved_temp=1; _anl_reserved_temp <= P-1;
	_anl_reserved_temp++)
    waitpid(-1, NULL, 0);
};

	// Calculate runtime
	t1 = -1;
	for (i=0; i < P-1; i++) {
	  if (gm->start[i] < t1) {
		t1 = gm->start[i];
	  }
	}
	
	t2 = 0;
	for (i=0; i < P-1; i++) {
	  if (gm->end[i] > t2) {
		t2 = gm->end[i];
	  }
	}

	printf("k: %u , M: %u \n",gm->k,gm->M); 
	printf("Total time: %u us\n",t2-t1);

	{
  fflush(stdout); 
  fflush(stderr); 
  exit(0);
}
	return 0;

}



