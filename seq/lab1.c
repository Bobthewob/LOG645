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

void problemeUn(int matrix[MATRIX_SIZE][MATRIX_SIZE], int n, int k)
{	
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 0; j < MATRIX_SIZE; j++)
		{
			usleep(1000);
			matrix[i][j] += (i + j) * k;			
		} 
	} 
	
	if (k < n)
	{
		problemeUn(matrix, n, ++k);
	} 	
}

void problemeDeux(int matrix[MATRIX_SIZE][MATRIX_SIZE], int n, int k)
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
	
	if (k < n)
	{
		problemeDeux(matrix, n, ++k);
	} 	
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Nombre de parametres invalides");
		return 0;
	}
	
	int matrix[MATRIX_SIZE][MATRIX_SIZE];
	int p = atoi(argv[2]);
	
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		for(int j = 0; j < MATRIX_SIZE; j++)
		{
			matrix[i][j] = p;
		} 
	}
	
	//Tiré de l'exemple du site du cours pour le minuteur
	double timeStart, timeEnd, executionTime;
	struct timeval tp;
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	
    if (atoi(argv[1]) == 1)
	{
		printf("Execution du premier probleme\n");
		problemeUn(matrix, atoi(argv[3]), 1);
	}
	else
	{
		printf("Execution du deuxieme probleme\n");
		problemeDeux(matrix, atoi(argv[3]), 1);
	} 
	
	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTime = timeEnd - timeStart;
	
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTime);
	
    return 0;
} 