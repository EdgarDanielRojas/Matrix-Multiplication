/*
 * Copyright (c) 2018 Edgar Daniel Rojas Vazquez
 *
 * @file    matrix.c
 *
 * @author  Edgar Daniel Rojas Vazquez & Abelardo López Lagunas
 *
 * @date    Thur Apr 11 21:00:00 CST 2018
 *
 * @brief   Program that multiplies two matrixes using threads
 *
 * References:
 *          The program uses parts of code from the producer consumer example
 *			and from the partial sum example.
 *
 * Restrictions:
 *          There is no validation in the code
 *
 * Revision history:
 *          Fri Apr 12 17:40:00 CST 2018 -- Added this header and finalized the program.
 *
 * @note    Homework for the TC2025 class. Some code is utilized from programs that the profesor
 *			has given us, that is why he is an author as well.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

pthread_mutex_t threadMutex=PTHREAD_MUTEX_INITIALIZER; 	// Initialization of our mutex that will be used to access shared arrays
pthread_mutex_t flagMutex=PTHREAD_MUTEX_INITIALIZER; 	// Initialization of mutex used as a type of flag
#define DIM 3											// We define a constant for the dimension of our array [DIM X DIM]
int arrays[2][DIM][DIM];								// Declaration of our two arrays that will be multiplied.
// Used to read integers from text files
int GetInt (FILE *fp) {
    int	c,i;	   /* Character read and integer representation of it */
    int sign = 1;

    do {
        c = getc (fp);                          /* Get next character */
        if ( c == '#' )	                          /* Skip the comment */
            do {
                c = getc (fp);
            } while ( c != '\n');
        if ( c == '-')
            sign = -1;
    } while (!isdigit(c) && !feof(fp));

    if (feof(fp)){
        return (EXIT_FAILURE);
    } else {
    /* Found 1st digit, begin conversion until a non-digit is found */
        i = 0;
        while (isdigit (c) && !feof(fp)){
            i = (i*10) + (c - '0');
            c = getc (fp);
        }

        return (i*sign);
    }
}

// Displays error message
void ErrorMsg (char * function, char *message){

    printf ("\nError in function %s\n", function);
    printf ("\t %s\n", message);
    printf ("The program will terminate.\n\n");
}

// Data structure that has variables needed to implement producer-consumer algorithm
struct prodcons {
  int buffer[DIM];   /* Data to store */
  pthread_mutex_t key;       /* Synchronization variable */
  int read, write;           /* Read/Write position */
  pthread_cond_t notEmpty;   /* Signal buffer NOT empty */
  pthread_cond_t notFull;    /* Signal buffer NOT full */
} buffer;

// Function that initializes variables of the previous declared structure
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

/* This is the function called by individual threads used to calculate the multiplication */
void *ixj(void *arg) {

	/* This section is used to declare variables that are used by each thread*/
	unsigned int * result; // Used to store result and return it to main thread. Must be stored in heap to avoid memory errors
	result = (unsigned int *) malloc (sizeof(unsigned int));
	unsigned int index; // Index is the name of the thread
	index = *((unsigned int *) (arg));
	int tempArrays[2][DIM]; // Arrays used to store the row and column to be multiplied
	int row = index/DIM; 	// The row in which the thread is calculating the result
	int column = index%DIM;	// The column in which the thread is calculating the result

	// The thread mutex is used to read the arrays
	// Not really necessary but in the specifications to use mutex
   	pthread_mutex_lock(&threadMutex);
   		// Critical Section
   		for(int i =0;i<DIM;i++){
	   		tempArrays[0][i]=arrays[0][row][i];
			tempArrays[1][i]=arrays[1][i][column];
   		}
	pthread_mutex_unlock(&threadMutex);

	// Calculate the result of the multiplication of the row and column
	for(int j=0;j<DIM;j++){
		*result += tempArrays[0][j]*tempArrays[1][j];
	}

	// Check to see if our thread is calculating a value in the diagonal. 
	// If true the result is sent to buffer
	if(index%(DIM+1)==0){
		Store(&buffer,*result);
	}

	// flagMutex is tried to see if a thread is calculating the sum of diagonals. 
	// If no thread is calculating, the thread takes over and starts consuming from our buffer
	// to get the sum.
	if(pthread_mutex_trylock(&flagMutex)==0){
		int sum=0;
		for(int j=0;j<DIM;j++){
			sum += Get(&buffer);
		}
		printf("\nSuma de la diagonal es %d\n", sum);
	}

	// Return result to main thread.
	pthread_exit((void *)result);
}

/* Main entry point creates several threads and assigns them a unique id */
int main(int argc, const char * argv[]) {
	// Variables are initialized which are used in main thread.
	const int MAXTHREADS = DIM*DIM;
  	pthread_t threads[MAXTHREADS]; // Thread array created
  	unsigned int threadName[MAXTHREADS]; // Array used to store thread name
  	unsigned int * ixjSum; // Variable where returned value from arrays will be stored.
  	// Call init to initialize buffer variables
  	init(&buffer); 
  	// Values are read into our arrays
  	FILE   *fp;
  	//Manual Option
  	if (argc < 2){
	  	for(int i=0;i<2;i++){
	  		printf("\n");
	  		for(int j=0;j<DIM;j++){
	  			for(int k=0;k<DIM;k++){
	  				printf("Ingrese la casilla [%d,%d] de la matriz %d: ", j+1,k+1,i+1);
	  				scanf("%d",&arrays[i][j][k]);
	  			}
	  			
	  		}
	  	}
  	} else{ //File option
        /* Open the file and check that it exists */
        fp = fopen (argv[1],"r");	  /* Open file for read operation */
        if (!fp) {                               /* There is an error */
            ErrorMsg("main","filename does not exist or is corrupted");
        } else {

       
                while (!feof(fp))
                {
                	int i = 0;
                    for(i = 0; ((i < DIM) && (!feof(fp))); i++)
                		{
                        /* Store values for matrices */
                        /* Values are stored from left to right as read from the file */
                        /* according to the size of the matrix */
                        for (int j = 0; j < DIM ; j++)
                        {
                            arrays[0][i][j] = GetInt(fp);
                        }

                        for (int j = 0; j < DIM ; j++)
                        {
                            arrays[1][i][j] = GetInt(fp);
                        }
                            
                    }

                    if(feof(fp) || i == DIM)
                      break;
                }
            }
        }

  /* Create the threads */
	  for (int i = 0; i < MAXTHREADS; i++){
	  	threadName[i] = i;
	  	if (pthread_create(&threads[i], NULL, ixj,&threadName[i]) != 0)
			perror("error creating thread.");
	  }

	  // An array to store incoming results is created
	  int resultingMatrix[DIM][DIM];
	  // Initialization of variables that store the row and column of the data received
	  int row = 0;
	  int column = 0;
	  /* Now wait for them in-order, this is inefficient */
	  for (unsigned int i = 0; i < MAXTHREADS; i++){
	    if (pthread_join ( threads[i], (void **)&ixjSum) != 0)
		  perror("error joining thread.");
		// Row and Column is calculated
		int row = i/DIM;
		int column = i%DIM;
		// Result saved in our matrix
		resultingMatrix[row][column] = *ixjSum;
		free (ixjSum);
	  }

	  // Matrix is printed in a nice format
	  printf("\nLa matriz resultante es la siguiente \n");
	  for (int row=0; row<DIM; row++)
		{
			printf("|");
		    for(int columns=0; columns<DIM; columns++)
		        {
		         	printf("   %5d   ", resultingMatrix[row][columns]);
		        }
		    printf("|\n");
	 	}
	  return 0;
}