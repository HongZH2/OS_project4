/*

taskset -c 1 ./problem_1 5 5
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
 
void cat_enter();
void cat_exit(); 
void dog_enter();
void dog_exit();
//void bird_enter();
//void bird_exit();
typedef enum {rest, playing} petstate;

pthread_mutex_t mutex;
pthread_cond_t cat_cond;
pthread_cond_t dog_cond;
pthread_cond_t brd_cond;
petstate *cat_status;
petstate *dog_status;
petstate *brd_status;

volatile int stop;
volatile int cats=0, dogs=0, birds=0;

int n_cats=0;
int n_dogs=0;
int n_birds=0; 

void play(void) {
  for (int i=0; i<10; i++) {
    assert(cats >= 0 && cats <= n_cats);
    assert(dogs >= 0 && dogs <= n_dogs);
    assert(birds >= 0 && birds <= n_birds);
    assert(cats == 0 || dogs == 0);
    assert(cats == 0 || birds == 0);
   }
}
 
/* create thread argument struct */
typedef struct _thread_data_t {
  int tid;
  int enter;
} thread_data_t;

/* bird function*/
void bird_enter(int k) {
  pthread_mutex_lock(&mutex);
  while (cats>0) {
    pthread_cond_wait(&brd_cond, &mutex);}
  birds++;  
  play(); 
  pthread_mutex_unlock(&mutex);
}

void bird_exit(int k) {
  pthread_mutex_lock(&mutex);
  birds--;
  if(dogs==0&&birds==0)
  {  
    pthread_cond_broadcast(&cat_cond);
  }
  pthread_mutex_unlock(&mutex);
}

void *bird_thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  int tid = data->tid;
  long int i=0;
  while(stop)
  {
    i++;
    bird_enter(tid);
    bird_exit(tid);
  }

  data->enter = i;
  pthread_exit(NULL);
}

/* dog function*/
void dog_enter(int k) {
  pthread_mutex_lock(&mutex);
  while (cats>0) {
    pthread_cond_wait(&dog_cond, &mutex);}
  dogs++;  
  play(); 
  pthread_mutex_unlock(&mutex);
}

void dog_exit(int k) {
  pthread_mutex_lock(&mutex);
  dogs--;
  if(dogs==0&&birds==0)
  {  
    pthread_cond_broadcast(&cat_cond);
  }
  pthread_mutex_unlock(&mutex);
}

void *dog_thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  int tid = data->tid;
  long int i=0;
  while(stop)
  {
    i++;
    dog_enter(tid);
    dog_exit(tid);
  }

  data->enter = i;
  pthread_exit(NULL);
}
/* dog function*/

/* cat function*/
void cat_enter(int k) {
  pthread_mutex_lock(&mutex);
  while (dogs>0 || birds>0) {
    pthread_cond_wait(&cat_cond, &mutex);}
  cats++;  
  play(); 
  pthread_mutex_unlock(&mutex);
}

void cat_exit(int k) {
  pthread_mutex_lock(&mutex);
  cats--;
  if(cats==0)
  {  
    pthread_cond_broadcast(&dog_cond);
    pthread_cond_broadcast(&brd_cond);
  }
  pthread_mutex_unlock(&mutex);
}

/*main thread function*/
void *cat_thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  int tid = data->tid;
  long int i=0;
  while(stop)
  {
    i++;
    cat_enter(tid);
    cat_exit(tid);
  }

  data->enter = i;
  pthread_exit(NULL);
}
/* cat function*/

 
int main(int argc, char *argv[]) {

  if(argc!=4)
  {  
    fprintf(stderr, "ERROR: Please enter 3 numbers\n");
    return EXIT_FAILURE;
  }
  /* convert string to int atoi()*/
  int NUM_CATS = atoi(argv[1]);
  int NUM_DOGS = atoi(argv[2]);
  int NUM_BIRS = atoi(argv[3]);

  if( NUM_CATS<0 || NUM_DOGS<0 || NUM_BIRS<0)
  {
    fprintf(stderr, "ERROR: Please enter positive numbers\n");
    return EXIT_FAILURE;
  }
  else if(NUM_CATS==0 && NUM_DOGS==0 && NUM_BIRS==0)
  {
    fprintf(stderr, "No pets in store.\n");
    return 0;
  }
   n_cats=NUM_CATS;
   n_dogs=NUM_DOGS;
   n_birds=NUM_BIRS; 
  /* create threads */
  pthread_t cat_thr[NUM_CATS];
  pthread_t dog_thr[NUM_DOGS];
  pthread_t brd_thr[NUM_BIRS];

  /* create a thread_data_t argument array */
  thread_data_t cat_tdata[NUM_CATS];
  thread_data_t dog_tdata[NUM_DOGS];
  thread_data_t brd_tdata[NUM_BIRS];

  int rc = 0;
  stop = 1;
  int i = 0;

  //mutex initial
  if(pthread_mutex_init(&mutex, NULL) != 0) {
    perror("pthread_mutex_init error");
    exit(1);
  }

  //conditional variable initial
  if(pthread_cond_init(&cat_cond, NULL)!= 0){
    perror("cat pthread_cond_init error");
    exit(1);
  }
   if(pthread_cond_init(&dog_cond, NULL)!= 0){
    perror("dog pthread_cond_init error");
    exit(1);
  }
   if(pthread_cond_init(&brd_cond, NULL)!= 0){
    perror("bird pthread_cond_init error");
    exit(1);
  }

  //cat thread
  for (i = 0; i < NUM_CATS; ++i) {
    cat_tdata[i].tid = i;
    if ((rc = pthread_create(&cat_thr[i], NULL, cat_thr_func, &cat_tdata[i]))) {
      fprintf(stderr, "error: cat pthread_create, rc: %d\n", rc);
      break;
    }
  }

  //dog thread
  for (i = 0; i < NUM_DOGS; ++i) {
    dog_tdata[i].tid = i;
    if ((rc = pthread_create(&dog_thr[i], NULL, dog_thr_func, &dog_tdata[i]))) {
      fprintf(stderr, "error: dog pthread_create, rc: %d\n", rc);
      break;
    }
  } 

  //dog thread
  for (i = 0; i < NUM_BIRS; ++i) {
    brd_tdata[i].tid = i;
    if ((rc = pthread_create(&brd_thr[i], NULL, bird_thr_func, &brd_tdata[i]))) {
      fprintf(stderr, "error: bird pthread_create, rc: %d\n", rc);
      break;
    }
  }   
  printf("Start playing\n");
  /*SLEEP*/
  sleep(10);
  stop = 0;
  long int sum_cat = 0;
  /* block until all threads complete */
  for (i = 0; i < NUM_CATS; ++i) {
    pthread_join(cat_thr[i], NULL);
    sum_cat+=cat_tdata[i].enter;
  }
  printf("Cats enter %ld times\n",sum_cat);
  sum_cat=0;
  /* block until all threads complete */
  for (i = 0; i < NUM_DOGS; ++i) {
    pthread_join(dog_thr[i], NULL);
    sum_cat+=dog_tdata[i].enter;
  }
  printf("Dogs enter %ld times\n",sum_cat);
  sum_cat=0;
  /* block until all threads complete */
  for (i = 0; i < NUM_BIRS; ++i) {
    pthread_join(brd_thr[i], NULL);
    sum_cat+=brd_tdata[i].enter;
  }
  printf("Birds enter %ld times\n",sum_cat);


  return EXIT_SUCCESS;
}
