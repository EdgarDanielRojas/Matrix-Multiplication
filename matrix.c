#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t threadMutex=PTHREAD_MUTEX_INITIALIZER;
#define DIM 3
int arrays[2][DIM][DIM];

void *diag(void *arg){
	pthread_t **threads= (pthread_t**)arg;
	unsigned int *value;
	int sum=0;
	for (unsigned int i = 0; i < DIM; i++){
    if (pthread_join (*threads[i*(DIM+1)], (void **)&value) != 0)
	  perror("error joining thread.");
	sum += *value;
	free (value);
  }
  printf("Suma de la diagonal: %d\n",sum );
  pthread_exit(NULL);
}

void *ixj(void *arg) {
unsigned int * result;
result = (unsigned int *) malloc (sizeof(unsigned int));
unsigned int index;
int tempArrays[2][DIM];
index = *((unsigned int *) (arg));
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
	//printf("Thread %d got result %d\n",index,result );
	pthread_exit((void *)result);
}

/* Main entry point creates several threads and assigns them a unique id */
int main(void) {
	const int MAXTHREADS = DIM*DIM;
  	pthread_t threads[MAXTHREADS];
  	unsigned int threadName[MAXTHREADS];
  	unsigned int * ixjSum;
  	for(int i=0;i<2;i++){
  		for(int j=0;j<DIM;j++){
  			for(int k=0;k<DIM;k++){
  				printf("\nIngrese la casilla [%d,%d] de la matriz %d: ", j+1,k+1,i+1);
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
  pthread_t diagThread;
  //pthread_create(&diagThread,NULL,diag,&threads);
  int resultingMatrix[DIM][DIM];
  /* Now wait for them in-order, this is inefficient */
  for (unsigned int i = 0; i < MAXTHREADS; i++){
    if (pthread_join ( threads[i], (void **)&ixjSum) != 0)
	  perror("error joining thread.");
	//printf("%s\n", "Right before adding values");
	int row = i/DIM;
	int column = i%DIM;
	//printf("Row %d and column %d\n",row,column );
	resultingMatrix[row][column] = *ixjSum;
	//printf("%d\n",resultingMatrix[i/DIM][i%DIM]);
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