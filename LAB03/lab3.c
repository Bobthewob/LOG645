#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

int err, numberOfProcess, processRank, m, n, np, nbproc;
double timeStart, timeEnd, executionTime, td, h;
struct timeval tp;

void printMatrix(double matrix[n][m][np], int iteration)
{
	printf("%s\n", "-----------------------------------------------------------------");
	for(int i = 0; i < n; i++) 
	{
		for(int j = 0; j < m; j++) 
		{
			printf("%lf ", matrix[i][j][iteration]);
		}
		
		printf("\n");
	} 
	printf("%s\n", "-----------------------------------------------------------------");
}

void initMatrix(double matrix[n][m][np])
{
	for (int k = 0; k < np; k++)
	{
	    for (int i = 0; i < n; i++)
	    {
	        for (int j = 0; j < m; j++)
	        {
	            matrix[i][j][k] = 0.0;
	        }
	    }
	}
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
	td = atof(argv[4]); //le temps discretise
	h = atof(argv[5]); //la taille d'un cote d'une subdivision
	nbproc = atoi(argv[6]); // le nombre de processus a utiliser

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

	double matrix[n][m][np];
	initMatrix(matrix);
	
	if(processRank == 0)
	{
		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
			{
				matrix[i][j][0] = i * (n - i - 1) * j * (m - j - 1);
			} 
		}		

		printf("%s\n", "Matrice après initialisation");
		printMatrix(matrix, 0);

		gettimeofday(&tp, NULL); 
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

		for(int k = 0; k < np; k++)
		{
			for(int i = 0; i < (n - 1); i++)
			{
				for(int j = 0; j < (m - 1); j++)
				{	
					double tmp = (td/(h * h));
					usleep(5);
					matrix[i][j][k + 1] = (1.0 - 4*td/h*h) * matrix[i][j][k] + 
										  (td/h*h) * (matrix[i - 1][j][k] + matrix[i + 1][j][k] + matrix[i][j - 1][k] + matrix[i][j + 1][k]);
				}
			}
		}

		gettimeofday(&tp, NULL);
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTime = timeEnd - timeStart;

		printf("%s\n", "Sequentielle");
		printMatrix(matrix, np);
		printf("Execution time: %f\n", executionTime);
	}

	MPI_Finalize();
    return 0;
} 