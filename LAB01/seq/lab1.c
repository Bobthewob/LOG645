#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"

#define MATRIX_SIZE 8

void printMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	for(int i = 0; i < MATRIX_SIZE; i++) 
	{
		for(int j = 0; j < MATRIX_SIZE; j++) 
		{
			printf("%d ", matrix[i][j]);
		}
		
		printf("\n");
	} 
} 

void initalizeMatrix(int p, int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 0; j < MATRIX_SIZE; j++)
		{
			usleep(1000);
			matrix[i][j] = p;
		} 
	}
}

void problemeUn(int matrix[MATRIX_SIZE][MATRIX_SIZE], int nbIterations, int k)
{	
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 0; j < MATRIX_SIZE; j++)
		{
			usleep(1000);
			matrix[i][j] += (i + j) * k;			
		} 
	} 
	
	if (k < nbIterations)
	{
		problemeUn(matrix, nbIterations, ++k);
	} 	
}

void problemeDeux(int matrix[MATRIX_SIZE][MATRIX_SIZE], int nbIterations, int k)
{	
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		usleep(1000); 
		matrix[i][0] += (i * k);		
	} 
	
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 1; j < MATRIX_SIZE; j++)
		{
			usleep(1000);
			matrix[i][j] += matrix[i][j - 1] * k;			
		} 
	} 
	
	if (k < nbIterations)
	{
		problemeDeux(matrix, nbIterations, ++k);
	} 	
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Nombre de parametres invalides");
		return 0;
	}
	
	int initialValue = atoi(argv[2]);
	int problem = atoi(argv[1]);
	int nbIterations = atoi(argv[3]);
	int matrix[MATRIX_SIZE][MATRIX_SIZE];
	
	initalizeMatrix(initialValue, matrix);
	
	//Tiré de l'exemple du site du cours pour le minuteur
	double timeStart, timeEnd, executionTime;
	struct timeval tp;
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	
    if (problem == 1)
	{
		problemeUn(matrix, nbIterations, 1);
	}
	else
	{
		problemeDeux(matrix, nbIterations, 1);
	} 
	
	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTime = timeEnd - timeStart;
	
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTime);
	
    return 0;
} 