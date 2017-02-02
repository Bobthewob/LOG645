#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include <omp.h>

#define MATRIX_SIZE 10

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
	double timeStart, timeEnd, executionTime;
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
				//usleep(50000); JE SAIS PAS POURQUOI IL MARCHE PAS
				matrix[i][j] += i + j;			
			} 
		} 
	}

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTime = timeEnd - timeStart;

	printf("%s\n", "Sequentielle");
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTime);
}

void problemeDeux(int valeurInitiale, int nombreIterations)
{
	double timeStart, timeEnd, executionTime;
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
				//usleep(50000); JE SAIS PAS POURQUOI IL MARCHE PAS
				if (j == 9)
				{
					matrix[i][0] += i;
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
	executionTime = timeEnd - timeStart;

	printf("%s\n", "Sequentielle");
	printMatrix(matrix);
	printf("Execution time: %f\n", executionTime);
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