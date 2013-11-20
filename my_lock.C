MAIN_ENV

typedef struct {
  unsigned int *start;
  unsigned int *end;
  int *a;
  int N,k,M;
  int numprocs;
  DEC(lock)
} GM;

GM *gm;

void benchmark(void) {
  register int i, j, p, q;
    int pid,N,k,M;
    N = gm->N;
    k = gm->k;
    M = gm->M;
    GET_PID(pid);
    //M = pid*(gm->M);
	p = 0;
	q = 0;
    CLOCK(gm->start[pid])

	for (i=0; i < N; i++) {
	    ACQUIRE(gm->lock)
		for (j=0; j < k; j++) { 
		  q++;
		}
		RELEASE(gm->lock)
		for (j=0; j < M; j++) {
		  p++;
		}
	}
	CLOCK(gm->end[pid])
	gm->a[pid] = p+q;
}

int main(int argc, char **argv) {
  int P, i;
  unsigned int t1, t2;
  unsigned int *start;
  unsigned int *end;
  int *a;
	MAIN_INITENV
	if (argc!=4) {
		printf("Usage: my_lock P k M\nAborting...\n");
		exit(0);
	}

	// Initialize global memory
	gm = (GM *)G_MALLOC(sizeof(GM));
	P = gm->numprocs = atoi(argv[1]);
	assert(P > 0);
	assert(P <= 8);
	INIT(gm->lock)

	// Initialize arrays in global memory
	start = gm->start = (unsigned int*)G_MALLOC(P*sizeof(unsigned int));
	end = gm->end = (unsigned int*)G_MALLOC(P*sizeof(unsigned int));
	a = gm->a = (int*)G_MALLOC(P*sizeof(int));

	// Initialize design parameters
	gm->N = 2000000;
	gm->k = atoi(argv[2]);
	gm->M = atoi(argv[3]);

	// Initialize processes per processor
	for(i = 0; i < P; i++) {
		CREATE(benchmark)
	}

	// Run the processes
	benchmark();
	WAIT_FOR_END(P-1);

	// Calculate runtime
	t1 = 4000000000;
	for (i=0; i < P; i++) {
	  if (gm->start[i] < t1) {
		t1 = gm->start[i];
	  }
	}
	
	t2 = 0;
	for (i=0; i < P; i++) {
	  if (gm->end[i] > t2) {
		t2 = gm->end[i];
	  }
	}

	printf("k: %u , M: %u \n",gm->k,gm->M); 
	printf("Total time: %u us\n",t2-t1);

	MAIN_END
	return 0;

}



