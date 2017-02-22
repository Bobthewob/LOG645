#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

int err, numberOfProcess, processRank, m, n, np, nbproc;
double timeStart, timeEnd, executionTime, td, h;
struct timeval tp;

void printMatrix(double matrix[np][n][m], int iteration)
{
	printf("%s\n", "-----------------------------------------------------------------");
	for(int i = 0; i < n; i++) 
	{
		for(int j = 0; j < m; j++) 
		{
			printf("%lf ", matrix[iteration][i][j]);
		}
		
		printf("\n");
	} 
	printf("%s\n", "-----------------------------------------------------------------");
}

void initMatrix(double matrix[np][n][m])
{
	for (int k = 0; k < np; k++)
	{
	    for (int i = 0; i < n; i++)
	    {
	        for (int j = 0; j < m; j++)
	        {
	            matrix[k][i][j] = 0.0;
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
	
	if(processRank == 0)
	{
		double matrix[np][n][m];	

		initMatrix(matrix);

		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
			{
				matrix[0][i][j] = i * (n - i - 1) * j * (m - j - 1);
			} 
		}		

		printf("%s\n", "Matrice après initialisation");
		printMatrix(matrix, 0);

		gettimeofday(&tp, NULL); 
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		int comp = 0;

		for(int k = 0; k < (np - 1); k++)
		{
			for(int i = 1; i < (n - 1); i++)
			{

				for(int j = 1; j < (m - 1); j++)
				{	
					comp++;

					usleep(5);
					matrix[k + 1][i][j] = (1.0 - 4.0*td/(h*h)) * matrix[k][i][j] + 
										  (td/(h*h)) * (matrix[k][i - 1][j] + matrix[k][i + 1][j] + matrix[k][i][j - 1] + matrix[k][i][j + 1]);
				}
			}
		}

		gettimeofday(&tp, NULL);
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTime = timeEnd - timeStart;

		printf("%s\n", "Sequentielle");
		printMatrix(matrix, np - 1);
		printf("Execution time: %f\n", executionTime);
	}

	MPI_Finalize();
    return 0;
} 