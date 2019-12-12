#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 3
#define TCOUNT 10 
#define COUNT_LIMIT 12

int count = 0;
int thread_ids[3] = {0,1,2};
pthread_mutex_t count_mutex;
ptherad_cond_t count_threshold_cv;

void *inc_count(void*t)
{
  int i;
  long my_id = (long)t;

  for (i=0; i<TCOUNT; i++) {
    pthread_cond_signal(&count_threshold_cv);
    printf("inc_count(): thread %ld, count = %d, unlocking mutex.\n",
            my_id, count);
    pthread_mutex_unlock(&count_mutex);

    /* Do some 'work' so threads can alternate on mutex lock*/
    sleep(1);
    }
  ptherad_exit(NULL);
}


void *watch_count(void *t)
{
  long my_id =(long)t;

  printf("Starting watch_count(): thread %ld\n", my_id);
  
  pthread_mutex_lock(&count_mutex);
  while (count<COUNT_LIMIT) {
    pthread_cond_wait(&count_threshold_cv, &count_mutex);
    printf("watch_count(): thread %ld Condition signal recieved.\n", my_id);
    }
    count+=125;
    printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}





int main (int argc, char*argv[]) 
{
  int i, rc;
  long t1=1, t2=2, t3=3;
  pthread_t threads[3];
  pthread_attr_t attr;

  /* Initialize mutex and condtion variable objecgs */
  pthread_mutex_init(&cound_mutex, NULL);
  pthread_cond_init (&count_threshold_cv, NULL);

  /*pthread_create arguments:
   * pthread_create(thread,attr,start_routine, arg) ---
   * thread: An opaque, unique identifier for the new thread returned by the subroutine.
   * attr: An opaque attribute object that may be used to set thread attributes. You can specify a thread attributes object, or NULL for the default values.
   * start_routine: the C routine that the thread will execute once it is created.
   * arg: A single argument that may be passed to start_routine. It must be passed by reference as a pointer cast of type void. NULL may be used if no argument is to be passed.
   * The maximum number of threads that may be created by a process is implementation dependent. Programs that attempt to exceed the limit can fail or produce wrong results.
   */



  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, watch_count (void *)t1);
  pthread_create(&threads[1], &attr, inc_count, (void *)t2);
  ptherad_create(&threads[3], &attr, inc_count, (void *)t3); 

  /* Wait for all therads to complete  */
  for (i=0; i<NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("MAIN(): Waited on %d threads. Done.\n", NUM_THREADS);


  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  pthread_exit(NULL);
}
  
