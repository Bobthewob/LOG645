#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

int err, numberOfProcess, processRank, m, n, np;
double timeStart, timeEnd, executionTime;
struct timeval tp;

void printMatrix(double matrix[n][m][np])
{
	printf("%s\n", "-----------------------------------------------------------------");
	for(int i = 0; i < n; i++) 
	{
		for(int j = 0; j < m; j++) 
		{
			printf("%lf ", matrix[i][j][np - 1]);
		}
		
		printf("\n");
	} 
	printf("%s\n", "-----------------------------------------------------------------");
}

int main(int argc, char *argv[])
{

	MPI_Init(&argc, &argv);

	if (err != MPI_SUCCESS)
	{
		printf("Erreur d'initialisation de MPI");
    	return 0;
	}

	if (argc != 7)
	{
		printf("Nombre de parametres invalides");
		return 0;
	}

	n = atoi(argv[1]); //nombre de lignes
	m = atoi(argv[2]); //nombre de colonnes
	np = atoi(argv[3]); //nombre de pas de temps
	int td = atoi(argv[4]); //le temps discretise
	int h = atoi(argv[5]); //la taille d'un cote d'une subdivision
	int nbproc = atoi(argv[6]); // le nombre de processus a utiliser

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

	gettimeofday(&tp, NULL); 
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

	double matrix[n][m][np];
	
	if(processRank == 0)
	{

		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
			{
				matrix[i][j][0] = i * (n - i - 1) * j * (m - j - 1);
			} 
		}

		printMatrix(matrix);

		for(int k = 0; k < np; k++)
		{
			for(int i = 0; i < (n - 1); i++)
			{
				for(int j = 0; j < (m - 1); j++)
				{	
					double tmp = (td/(h * h));
					matrix[i][j][k + 1] = ((1 - 4 * tmp) * matrix[i][j][k]) + tmp * (matrix[i - 1][j][k] + matrix[i + 1][j][k] + matrix[i][j - 1][k] + matrix[i][j + 1][k]);
				}
			}
		}

		printMatrix(matrix);
	}

	MPI_Finalize();
    return 0;
} 