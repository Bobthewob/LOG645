#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include <omp.h>

#define MATRIX_SIZE 10
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void initalizeMatrix(int p, int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 0; j < MATRIX_SIZE; j++)
		{
			matrix[i][j] = p;
		} 
	}
}

void printMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	printf("%s\n", "-----------------------------------------------------------------");
	for(int i = 0; i < MATRIX_SIZE; i++) 
	{
		for(int j = 0; j < MATRIX_SIZE; j++) 
		{
			printf("%d ", matrix[i][j]);
		}
		
		printf("\n");
	} 
	printf("%s\n", "-----------------------------------------------------------------");
}

void problemeUn(int valeurInitiale, int nombreIterations)
{
	double timeStart, timeEnd, executionTimeSeq, executionTimePar, acceleration;
	struct timeval tp;
	int matrix[MATRIX_SIZE][MATRIX_SIZE];	
	initalizeMatrix(valeurInitiale, matrix);

	//Tiré de l'exemple du site du cours pour le minuteur
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

	//Partie séquentielle
	for (int k = 1; k <= nombreIterations; k++)
	{	
		for(int i = 0; i < MATRIX_SIZE; i++)
		{
			for(int j = 0; j < MATRIX_SIZE; j++)
			{
				usleep(50000);
				matrix[i][j] += i + j;			
			} 
		}
	}

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTimeSeq = timeEnd - timeStart;

	printf("%s\n", "Sequentielle");
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTimeSeq);

	initalizeMatrix(valeurInitiale, matrix);

	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

	//Partie parallel
	#pragma omp parallel for schedule(dynamic) 
	for(int i = 0; i < (MATRIX_SIZE * MATRIX_SIZE); i++)
	{
		int y = i % MATRIX_SIZE;
		int x = i / MATRIX_SIZE;
		
		for (int k = 1; k <= nombreIterations; k++)
		{
			usleep(50000);
			matrix[y][x] += y + x;	
		}	
	}

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTimePar = timeEnd - timeStart;
	acceleration = executionTimeSeq / executionTimePar;

	printf("%s\n", "parallel");
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTimePar);
	printf("Acceleration: %f\n", acceleration);
}

void problemeDeux(int valeurInitiale, int nombreIterations)
{
	double timeStart, timeEnd, executionTimeSeq, executionTimePar, acceleration;
	struct timeval tp;
	int matrix[MATRIX_SIZE][MATRIX_SIZE];

	initalizeMatrix(valeurInitiale, matrix);
	
	//Tiré de l'exemple du site du cours pour le minuteur
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	
	//Partie séquentielle
	for (int k = 1; k <= nombreIterations; k++)
	{
		for(int j = MATRIX_SIZE - 1; j >= 0; j--)
		{	
			for(int i = 0; i < MATRIX_SIZE; i++)
			{
				usleep(5000);

				if(j == MATRIX_SIZE - 1)
				{
					matrix[i][j] += i;
				}
				else
				{
					matrix[i][j] += matrix[i][j + 1];	
				}
			} 
		}
	}

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTimeSeq = timeEnd - timeStart;

	printf("%s\n", "Sequentielle");
	printMatrix(matrix);

	printf("Execution time: %f\n", executionTimeSeq);
	
	initalizeMatrix(valeurInitiale, matrix);	

	//Tiré de l'exemple du site du cours pour le minuteur
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	
	//Partie parallele
	for (int k = 1; k <= nombreIterations; k++)
	{
 		for(int j = MATRIX_SIZE - 1; j >= 0; j--)
 		{	
 			#pragma omp parallel for schedule(dynamic)
 			for(int i = 0; i < MATRIX_SIZE; i++)
 			{
 				usleep(5000);
 
 				if(j == MATRIX_SIZE - 1)
 				{	
 					matrix[i][j] += i;
 				}
 				else
 				{
 					matrix[i][j] += matrix[i][j + 1];	
 				}
 			} 
 		}
 	}

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTimePar = timeEnd - timeStart;
	acceleration = executionTimeSeq / executionTimePar;

	printf("%s\n", "Parallele");
	printMatrix(matrix);

	printf("Execution time: %f\n", executionTimePar);
	printf("Acceleration: %f\n", acceleration);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Nombre de parametres invalides");
		return 0;
	}

	int valeurInitiale = atoi(argv[2]);
	int problem = atoi(argv[1]);
	int nbIterations = atoi(argv[3]);

	if (problem == 1)
	{
		problemeUn(valeurInitiale, nbIterations);
	}
	else
	{
		problemeDeux(valeurInitiale, nbIterations);
	}

    return 0;
} 