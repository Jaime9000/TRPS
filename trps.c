#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <syscall.h>
#include <assert.h>
#include <errno.h>
#include  <time.h>


#define NUM_THREADS 2 
#define MAX_THREADS 1

int chi_0 = 0, chi_1 = 0;
int ties =0;
int rnds = 0;
//vars
int cmd = 0 ; ///BUFFFER values 
pthread_cond_t cv1;  
pthread_mutex_t cmutex; 
int cstatus[] = {0,0}; //count 

//vars for throw
int throws[] = {0,0};
pthread_mutex_t tmutex;
pthread_cond_t cv2;
int tstatus[] = {0,0};




void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("What's good? It's thread #%ld\n", tid);
   pthread_exit(NULL);
}


int throw(){
    int randnum;
    srand(time(NULL));
    randnum = rand() %3;
    return randnum;
}
void *player(void *arg){
  int *num = (int *)arg;
  int player_num = *num;
  int loopy = 1; //start with GO
  
  while(loopy == 1){
      pthread_mutex_lock(&cmutex);
      
      while(cstatus[player_num] == 0) { //buf is empty 
          ///printf("Pthread waiting\n");
          pthread_cond_wait(&cv1, &cmutex);
      }
     
      
      loopy = cmd;
      cstatus[player_num]--;
      
      pthread_mutex_unlock(&cmutex);
      if(loopy == 0) {printf("\nThread %d: now exiting\n",player_num); break; }
      
      pthread_mutex_lock(&tmutex);

      
    
      int rndThrow = rand() %3;
      //Now pthreads thorws
      if(rndThrow == 0){
        printf("RandThrow is (%d) %s for thread p%d\n",rndThrow, "ROCK", player_num);
      }

      if(rndThrow ==1){
        printf("RandThrow is (%d) %s for thread p%d\n",rndThrow, "PAPER", player_num);
      }

      if(rndThrow==2){
        printf("RandThrow is (%d) %s for thread p%d\n",rndThrow, "SICCORS", player_num);
      }

      //printf("RandThrow is %d for thread p%d\n",rndThrow, player_num); 
      throws[player_num] = rndThrow;
      tstatus[player_num] = 1;
  
      pthread_cond_signal(&cv2);
      pthread_mutex_unlock(&tmutex);
  }
  
  pthread_exit(NULL); 
  
}

int winner(int throw1, int throw2){ ///had some trouble with passing args & the matrix logic so used ifs
    
    if( throw1 == 0 && throw2 == 1){
        printf("Child p1 wins, paper covers rock\n");
        chi_1++;
    } 
    
    if( throw1 == 0 && throw2 ==2){
        printf("Child p0 wins, rock breaks siccors\n");
        chi_0++;
    }

    if( throw1 == 1 && throw2 == 0){
        printf("Child p1 wins, paper covers rock\n");
        chi_1++;
    }

    if  (throw1 == 1 && throw2 == 2){
        printf("Child p1 wins, siccors cut paper\n");
        chi_1++;
    }

    if (throw1 == 2 && throw2 == 0) {
        printf("Child p1 wins, rock breaks siccors\n");
        chi_1++;
    }

    if (throw1 ==2 && throw2 == 1){
        printf("Child p0 wins, siccors cuts paper\n");
        chi_0++;
    }

  return 0;
}

int results(){
    
    if( (chi_0 + chi_1 + ties) == rnds){
      printf("------------------------\n");
      printf("Results:\n Child_0: %d\n Child_1: %d\n", chi_0, chi_1);
      printf(" Ties   : %d\n ", ties);
    
      if(chi_0 > chi_1){
        printf("Child_0, p0, Wins!\n");        
      }
  
      else if(chi_0 < chi_1){
        printf("Child_1, p1, Wins!\n");
      } 
    } 
   
  return 0;
}

//mustex and cond destroy at end of functions
void ref(void *arg) {
    int i, N;
    N = rnds;
    //printf("Inside Ref\n");
    printf("Rounds: %d\n", N);
    for(i =0; i < N; i++){
        printf("------------------------\n");
        printf("Round %d:\n", i+1);
        pthread_mutex_lock(&cmutex);
        
        cmd = 1;
        cstatus[0] = 1;
        cstatus[1] = 1;

        pthread_cond_broadcast(&cv1); //Now signl both waiting threads
        pthread_mutex_unlock(&cmutex);
    
        pthread_mutex_lock(&tmutex);

        while((tstatus[0] == 0) || (tstatus[1] == 0)) { 
            pthread_cond_wait(&cv2, &tmutex);
        }

        if(throws[0] == throws[1]){
            printf("Game is a tie!\n");
            ties++;
        }
        else{
            winner(throws[0], throws[1]);
            
        }
        
        results();

        tstatus[0] = 0;   //reset the throws
        tstatus[1] = 0;   //reset the throws
        

        pthread_mutex_unlock(&tmutex);
    }

    pthread_mutex_lock(&cmutex);
    cmd = 0;
    cstatus[0] = 1;
    cstatus[1] = 1;
    pthread_cond_broadcast(&cv1);
    pthread_mutex_unlock(&cmutex);

    pthread_exit(NULL);
}




int main (int argc, char *argv[]) {
  if(argc < 2) {
    printf("You must provide a second arg for the # of rounds\n");
    return 1;
  }
  
  srand(time(NULL));

  pthread_t child1;
  pthread_t child2;
  int c;
  int p0 = 0, p1 = 1;
  long reffery;
  rnds = atoi(argv[1]); 
  
  pthread_mutex_init(&cmutex, NULL);
  pthread_mutex_init(&tmutex, NULL);
  pthread_cond_init(&cv1, NULL);
  pthread_cond_init(&cv2, NULL);





    c = pthread_create(&child1, NULL, player, &p0);
    if(c != 0){
      perror("ERROR");
    } 
    
    c = pthread_create(&child2, NULL, player, &p1);
    if(c != 0){
      perror("ERROR");
    }
    printf("REF: creating child thread, LWP id is %d\n", (unsigned int)child1);
    printf("REF: creatong chlid thread, LWP id is %d\n", (unsigned int) child2);
    //printf("Past pthread_create*2\n");

    ref(NULL);
  
  /*"last thang in main y'all"*/
    pthread_join(child1, NULL);
    pthread_join(child2, NULL);
    pthread_mutex_destroy(&cmutex);
    pthread_cond_destroy(&cv1);
    pthread_mutex_destroy(&tmutex);
    pthread_cond_destroy(&cv2);
}
