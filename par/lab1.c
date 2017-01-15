#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

#define MATRIX_SIZE 8

int err, numberOfProcess, processRank;
double timeStart, timeEnd, executionTime;
struct timeval tp;

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

void aggregateAndPrintValues(int currentValue, int i, int j)
{
	if(processRank == 0) //processeur consideré comme le serveur , va recevoir les données des autres processeurs et créer la matrice
	{		
		int buffToRecv[3];
		int matrix[MATRIX_SIZE][MATRIX_SIZE];
		int nbOfRecvValue = 0;
		MPI_Status status;

		matrix[0][0] = currentValue;

		while(nbOfRecvValue < (numberOfProcess - 1))
		{
			MPI_Recv(buffToRecv, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			matrix[buffToRecv[1]][buffToRecv[2]] = buffToRecv[0];
			++nbOfRecvValue;
		}
	
		printMatrix(matrix);

		gettimeofday(&tp, NULL);
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTime = timeEnd - timeStart;
		printf("Execution time: %f\n", executionTime);
	}
	else  //processeur consideré comme un client , va envoyer sa valeur de cellule au processeur serveur
	{
		int buffToSend[3];
		buffToSend[0] = currentValue;
		buffToSend[1] = i;
		buffToSend[2] = j;

		MPI_Send(buffToSend , 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
		
}

void problemeUn(int oldValue, int nbIterations, int k, int i, int j)
{
	int currentValue;

	usleep(1000);
	currentValue = oldValue + (i + j) * k;			
	
	if (k < nbIterations)
	{
		problemeUn(currentValue, nbIterations, ++k, i, j);
	} 	
	else
	{
		aggregateAndPrintValues(currentValue, i, j);
	}
}

void problemeDeux(int oldValue, int nbIterations, int k, int i, int j)
{	
	int currentValue;

	if(j == 0) 
	{
		usleep(1000);
		currentValue = oldValue + (i * k);

		int buffToSend[1];
		buffToSend[0] = currentValue;

		MPI_Send(buffToSend , 1, MPI_INT, processRank + MATRIX_SIZE, 0, MPI_COMM_WORLD);

	}
	else if((j > 0) && (j < 7)) 
	{
		int buffToRecv[1];
		MPI_Status status;
	
		MPI_Recv(buffToRecv, 1, MPI_INT, processRank - MATRIX_SIZE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		usleep(1000);
		currentValue = oldValue + (buffToRecv[0] * k);

		int buffToSend[1];
		buffToSend[0] = currentValue;

		MPI_Send(buffToSend , 1, MPI_INT, processRank + MATRIX_SIZE, 0, MPI_COMM_WORLD);
	}
	else
	{
		int buffToRecv[1];
		MPI_Status status;
	
		MPI_Recv(buffToRecv, 1, MPI_INT, processRank - MATRIX_SIZE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		usleep(1000);
		currentValue = oldValue + (buffToRecv[0] * k);
	}
	
	if (k < nbIterations)
	{
		problemeDeux(currentValue, nbIterations, ++k, i, j);
	} 	
	else
	{
		aggregateAndPrintValues(currentValue, i, j);
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
	
	MPI_Init(&argc, &argv);

	if (err != MPI_SUCCESS)
	{
		printf("Erreur d'initialisation de MPI");
    	return 0;
	}

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

	//Tiré de l'exemple du site du cours pour le minuteur
	if(processRank == 0)
	{
		gettimeofday(&tp, NULL); 
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	}

	if (problem == 1)
	{
		problemeUn(initialValue, nbIterations, 1, processRank % 8, processRank / 8);
	}
	else
	{
		problemeDeux(initialValue, nbIterations, 1, processRank % 8, processRank / 8);
	} 

	MPI_Finalize();
    return 0;
} 