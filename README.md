# Matrix-Multiplication
Matrix Multiplication using multiple threads for advanced programming class

## Introduction
  The objective of this practice is to use threads effectively to solve the multiplication of two M x M matrices. You must use N threads that compute the multiplication of row i X column j of two square matrices and then send their results to the main thread using thread synchronization. The program must also use one of the running threads to calculate  There are various approaches to implementing solutions, as is custom in computer science. Some are better than others, while some are mathematically the most optimal solution. Sadly, as accustomed, I did my solution in a hurry and it has a lot of room to be optimized, but I will explain what I did in the next section. 

## Solution
  The first step in the solution is to get our two matrixes, this can either be done manually or through a text file (.txt). Once we have the information of both matrices, we create threads for each of the values to be calculated. In the case of a M x M matrix, M^2 threads are created to compute the solution. Here is where jobs are seperated, after the creation the master thread waits for each of the threads in a sequenced fashion while the child threads do their workload. The workload consist of computing it's row and column (many classmates implemented this, I want to mention it was my idea), using those values to access the shared matrixes (using mutex of course, these are not necessary, but it was specified to use these in any shared structures) and copy the values it needs to compute, and finally computing it's corresponding section of the result matrix. That is the standard workload for all threads, and I would like to point one thing out. Why not calculate the value in the critical section? My reasoning is that in huge matrixes that would result in a lot of waiting time, it is not the same to copy a value than to multiply two values. That way, the threads read the info they need and then let the rest of the threads read while it computes it's value. From this point on there are various things that can happen. First, if the thread belongs to the diagonal of the matrix, it pushes it's value to a shared buffer that can be read from the thread that is computing the sum. This is done using the producer-consumer model seen in class. Next the thread performs a trylock that just serves as a flag and isn't used for accessing a critical section. I would like to mention that this was also an idea that I got when the teacher was explaining possible solutions and was copied by some classmates. The first thread to finish will be able to lock our flag mutex and perform the sum of the diagonal, while the rest of the threads ignore this section completely. Finally the threads exit with the value that they computed.

## Observations
First of all I would like to say that I know that the sequential read of finishing threads is inefficient, and this could be solved with something similar to the producer-consumer model where threads tell the main thread when they are finished. This makes it so that you don't wait for slower threads but in reality with the small computations that we are doing I consider that it would just add extra steps that in the end will make it do the same time. The only thing that is a bit inefficient is needing to wait for the thread that is computing the sum of the diagonal, this is because it waits until all of the threads that need to produce a diagonal sum are finished. What would be more efficient would be to make the last thread to finish compute this, as it does not need to wait for the rest of the threads, and the main thread is probably already waiting for it so that would lower our execution time in bigger matrixes. Finally I would like to mention that my algorithm is O(n^4) as I create a thread for each value of the matrix (n * n) and then each of those realizes n^2 computations. I thought this could be made into a O(n^3) solution by reducing the number of threads to only N but then the computations made by these would increase so it would be the same complexity. I would just like to say that I am ashamed of not being able to make this complexity smaller, or maybe I am calculating incorrectly. 

#Compilation Instructions
##Linux compilation

gcc matrix.c -pthread -o matrix

##MAC OSX compilation

gcc matrix.c -o matrix

#Running the program
El programa se puede correr insertando un archivo de texto o sin archivo de texto

./matrix archivo.txt

./matrix 
