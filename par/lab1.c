#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

#define MATRIX_SIZE 8

int err, numberOfProcess, processRank;

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
			matrix[i][j] = p;
		} 
	}
}

void problemeUn(int matrix[MATRIX_SIZE][MATRIX_SIZE], int nbIterations)
{
	if (processRank == 0)
	{
		matrix[0][0] = nbIterations;
		
		printMatrix(matrix);
	}
	else
	{

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
	
	err = MPI_Init(&argc, &argv);

	if (err != MPI_SUCCESS)
	{
		printf("Erreur d'initialisation de MPI");
    	return 0;
	}

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

	//Tiré de l'exemple du site du cours pour le minuteur
	double timeStart, timeEnd, executionTime;
	struct timeval tp;
	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

	if (problem == 1)
	{
		problemeUn(matrix, nbIterations);
	}
	else
	{
		printf("Execution du deuxieme probleme\n");
	} 

	gettimeofday(&tp, NULL);
	timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	executionTime = timeEnd - timeStart;
	
	if (processRank == 0)
	{
		printf("Execution time: %f\n", executionTime);
	}

	err = MPI_Finalize();
	
    return 0;
} 