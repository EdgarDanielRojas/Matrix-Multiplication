#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t threadMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flagMutex=PTHREAD_MUTEX_INITIALIZER;
#define DIM 3
int arrays[2][DIM][DIM];

struct prodcons {
  int buffer[DIM];   /* Data to store */
  pthread_mutex_t key;       /* Synchronization variable */
  int read, write;           /* Read/Write position */
  pthread_cond_t notEmpty;   /* Signal buffer NOT empty */
  pthread_cond_t notFull;    /* Signal buffer NOT full */
} buffer;

void init(struct prodcons * b)
{
  pthread_mutex_init(&b->key, NULL);
  pthread_cond_init(&b->notEmpty, NULL);
  pthread_cond_init(&b->notFull, NULL);
  b->read = 0;
  b->write = 0;
}

/* This function stores a data item in the buffer */
void Store(struct prodcons * b, int data)
{
  pthread_mutex_lock(&b->key);
  /* Wait until the buffer is not full */
  while ((b->write + 1) % DIM == b->read) {
    pthread_cond_wait(&b->notFull, &b->key);
  }
  /*
   * This function suspends the thread until notFull is true
   * note that it will acquire the key when the thread resumes
   */
  b->buffer[b->write] = data;
  b->write++;
  if (b->write >= DIM) b->write = 0;
  /* Signal buffer not full */
  pthread_cond_signal(&b->notEmpty);
  pthread_mutex_unlock(&b->key);
}

/* This function reads and removes a data item from the buffer */
int Get (struct prodcons * b)
{
  int data;
  pthread_mutex_lock(&b->key);
  /* Wait until the buffer is not empty */
  while (b->write == b->read) {
    pthread_cond_wait(&b->notEmpty, &b->key);
  }
  /*
   * This function suspends the thread until notEmpty is true
   * it also acquires the key when the thread resumes. Read
   * the data item and advance the pointer.
   */
  data = b->buffer[b->read];
  b->read++;
  if (b->read >= DIM) b->read = 0;
  /* Signal buffer not full */
  pthread_cond_signal(&b->notFull);
  pthread_mutex_unlock(&b->key);
  return (data);
}


void *ixj(void *arg) {
	unsigned int * result;
	result = (unsigned int *) malloc (sizeof(unsigned int));
	unsigned int index;
	index = *((unsigned int *) (arg));
	int tempArrays[2][DIM];
	int row = index/DIM;
	int column = index%DIM;
   	pthread_mutex_lock(&threadMutex);
   		for(int i =0;i<DIM;i++){
	   		tempArrays[0][i]=arrays[0][row][i];
			tempArrays[1][i]=arrays[1][i][column];
   		}
	pthread_mutex_unlock(&threadMutex);
	for(int j=0;j<DIM;j++){
		*result += tempArrays[0][j]*tempArrays[1][j];
	}

	if(index%(DIM+1)==0){
		//printf("Thread %d is in diagonal, storing my result\n",index );
		Store(&buffer,*result);
		//printf("Thread %d finished storing result\n",index );
	}


	if(pthread_mutex_trylock(&flagMutex)==0){
		int sum=0;
		//printf("Thread %d is adding the diagonal\n", index);
		for(int j=0;j<DIM;j++){
			sum += Get(&buffer);
			//printf("Diagonal read from spot %d sum is at %d\n",j,sum );
		}
		printf("\nSuma de la diagonal es %d\n", sum);
	}
	//printf("Thread %d got result %d\n",index,result );
	pthread_exit((void *)result);
}

/* Main entry point creates several threads and assigns them a unique id */
int main(void) {
	const int MAXTHREADS = DIM*DIM;
  	pthread_t threads[MAXTHREADS];
  	unsigned int threadName[MAXTHREADS];
  	unsigned int * ixjSum;
  	init(&buffer);
  	for(int i=0;i<2;i++){
  		printf("\n");
  		for(int j=0;j<DIM;j++){
  			for(int k=0;k<DIM;k++){
  				printf("Ingrese la casilla [%d,%d] de la matriz %d: ", j+1,k+1,i+1);
  				scanf("%d",&arrays[i][j][k]);
  			}
  			
  		}
  	}
  /* Create the threads */
	  for (int i = 0; i < MAXTHREADS; i++){
	  	threadName[i] = i;
	  	if (pthread_create(&threads[i], NULL, ixj,&threadName[i]) != 0)
			perror("error creating thread.");
	  }
	  int resultingMatrix[DIM][DIM];
	  int row = 0;
	  int column = 0;
	  /* Now wait for them in-order, this is inefficient */
	  for (unsigned int i = 0; i < MAXTHREADS; i++){
	    if (pthread_join ( threads[i], (void **)&ixjSum) != 0)
		  perror("error joining thread.");
		int row = i/DIM;
		int column = i%DIM;
		//printf("Received info from thread %d\n",i);
		resultingMatrix[row][column] = *ixjSum;
		free (ixjSum);
	  }

	  printf("\nLa matriz resultante es la siguiente \n");
	  for (int row=0; row<DIM; row++)
		{
		    for(int columns=0; columns<DIM; columns++)
		        {
		         	printf("%d     ", resultingMatrix[row][columns]);
		        }
		    printf("\n");
	 	}
	  return 0;
}