
#include <stdio.h>
#include <stdlib.h>
#include <shmem.h>
#include <shmemx.h>
#include "../support/rtc.h"

#define HANDLER_ID_REP 420
#define HANDLER_ID_REQ 421

volatile int count = 0; 
long pSync[SHMEM_BARRIER_SYNC_SIZE];

void sample_req_handler(void *buf, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	shmemx_am_reply(HANDLER_ID_REP, buf, nbytes, token);
}


void sample_rep_handler(void *buf, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
}


int main(int argc, char **argv)
{
  const long int ITER_CNT = 100;
  const long int MAX_MSG_SIZE = 1048576;
  int* source_addr;
  int peer;
  long int i=0,j=0, buff_size; 
  long long int start_time, stop_time, res;
  double time;

  shmem_init();

  int pe_id = shmem_my_pe();
  source_addr = (int*) malloc(MAX_MSG_SIZE);

  if(pe_id == 1) {
      if(shmem_n_pes()!=4)
      	fprintf(stderr,"Num PEs should be ==4");
      printf("#Message Cnt;Time(s);MR(msgs/sec)\n");
  }

  if (pe_id==1)
	  peer = 3;
  else if(pe_id==3)
	  peer = 1;
  get_rtc_res_(&res);

  for (i = 0; i < SHMEM_BARRIER_SYNC_SIZE; i += 1){
          pSync[i] = SHMEM_SYNC_VALUE;
  }

  /* Collective operation: Implicit barrier on return from attach */
  shmemx_am_attach(HANDLER_ID_REQ, &sample_req_handler);
  shmemx_am_attach(HANDLER_ID_REP, &sample_rep_handler);
  shmem_barrier_all();
  if(pe_id == 1 || pe_id == 3) {

 	 for(buff_size=1; buff_size<=MAX_MSG_SIZE; buff_size*=2) {
  	     shmem_barrier(1,1,2,pSync);
 	     get_rtc_(&start_time);
 	     for(j=1;j<=ITER_CNT;j++) {
 	     	  shmemx_am_request(peer, HANDLER_ID_REQ, source_addr, buff_size);
 	     	  shmemx_am_quiet();
 	     }
  	     shmem_barrier(1,1,2,pSync);
 	     get_rtc_(&stop_time);
 	     time = (stop_time - start_time)*1.0/(double)res/ITER_CNT;
 	     if(pe_id == 1) {
 	    	 printf("%20ld;%20.12f;%20.12f\n", 
 	                 buff_size, time, (double)buff_size/time);
 	     }
 	     fflush(stdout);
 	 }
  }

  shmem_barrier_all();
  shmem_finalize();

}

