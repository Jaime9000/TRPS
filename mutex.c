#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Struct which has the necissary info to allow the function 'dotprod'
 * to acccess its input and access its input data and place its output
 * into the structure
 */

typedef struct 
  {
    double *a;
    double *b;
    double sum;
    int veclen;
  } DOTDATA;

/* Define global vars and a mutex */

#define NUMTHREDS 4
#define VECLEN 100
    DOTDATA dotstr;
    pthread_t callThd[NUMTHREDS];
    pthread_mutex_t mutexsum; 

/* Function dotprod()
 * Activaited when thread is created.
 * all info from DOTDATA
 * all output is written to DOTDATA
 * multithread benefit:
 *    when thread created we pass a single argument to the activaited function
 *    -this argmuent usually a thread number
 *    -all other info that the function requres is accessed through the gloablly acessible structure
 */

void *dotprod(void *arg)
{
    /* Local Vars for use/convenience*/
    int i, start, end, len;
    long offset;
    double mysum, *x, *y;
    offset = (long)arg;

    len = dotstr.veclen;
    start = offset*len;
    end = start + len;
    x = dotstr.a;
    y = dotstr.b;

    /*
     * Perform the dot procudt and assign result
     * to the appropriate variable in the structure.
     */

    mysum = 0;
    for (i =start; i < end; i++)
      {
        mysum += (x[i] * y[i]);
      }
    
    /*
     * Lock a mutex prior to updating the value in the shared
     * structure, and unlck it upon updating.
     */
    pthread_mutex_lock (&mutexsum);
    dotstr.sum += mysum;
    pthread_mutex_unlock (&mutexsum);

    pthread_exit((void*) 0);

}

/*
 * The main program creates the threads which effectively carryout the work,
 * print our the result upon completion.
 * First the input data is created, before threads are created.
 * Due to the fact that all threads update a 'shated structure', we need
 * a mutex for mutual exclusion. The main thread needs to wait for all to complete,
 * it waits for each one of the threads.
 * We specify a thread attribute value that allow the main thread to join with the thread it creates.
 *  NOTE-handles are 'freed up' whe they are nolonger needed.
 */

int main (int argc, char *argv[])
{
    long i;
    double *a, *b;
    void *status;
    pthread_attr_t attr;

    /* Assign storage and initialize values*/
    a = (double*) malloc (NUMTHREDS*VECLEN*sizeof(double));
    b = (double*) malloc (NUMTHREDS*VECLEN*sizeof(double));

    for (i=0; i<VECLEN*NUMTHREDS; i++);
      {
      a[i] = 1.0;
      b[i] = a[1];
      }

    dotstr.veclen = VECLEN;
    dotstr.a = a;
    dotstr.b = b;
    dotstr.sum = 0;

    pthread_mutex_init(&mutexsum, NULL);

    /* Create threads to preform the dotproduct */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(i=0; i<NUMTHREDS; i++)
    {
    /*
     * Each Thread works on a diffrent set of data. The offset is specified by 'i'.
     * The size of the data for each thread is indicaited by VECLEN.
     */
    pthread_create(&callThd[i], &attr, dotprod, (void *)i);
    }

    pthread_attr_destroy(&attr);

    /* Wait on the other threads */
    for(i=0; i<NUMTHREDS; i++)
      {
      pthread_join(callThd[i], &status);
      }
    /* After joining, print out the results and cleanup */
    printf("Sum = %f\n", dotstr.sum);
    free (a);
    free (b);
    pthread_mutex_destroy(&mutexsum);
    pthread_exit(NULL);
}









