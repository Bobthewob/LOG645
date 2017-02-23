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

	int realN = n - 2; // we dont want to do any process on the false cells (the cells always equal to 0)
	int realM = m - 2;

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);	
	
	double numberOfCells = (realN*realM);

	double nbCellPerProcessor = numberOfCells / nbproc;

	int procT1 = (nbCellPerProcessor - ((int) nbCellPerProcessor)) * nbproc;
	
	int mappingMatrix[n][realM];
	double matrix[np][realN][realM];	

	int currentProc = 0;
	int i = 0;

	while(i < numberOfCells)
	{
		if(currentProc < procT1)
		{
			for (int j = 0; j < ((int) nbCellPerProcessor) + 1; ++j)
			{
				mappingMatrix[i / realN][i % realM] = currentProc;
				++i;
			}
			++currentProc;
		}
		else
		{
			for (int j = 0; j < ((int) nbCellPerProcessor); ++j)
			{
				mappingMatrix[i / realN][i % realM] = currentProc;
				++i;
			}
			++currentProc;
		}
	}	

	//k = 0
	for (int i = 0; i < realN; i++)
	{
		for (int j = 0; j < realM; j++)
		{
			if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
			{
				matrix[0][i][j] = (i + 1) * (n - i - 2) * (j + 1) * (m - j - 2);

				if(np > 1)
				{
					double buffToSend1[3];
					buffToSend1[0] = matrix[0][i][j];
					buffToSend1[1] = i;
					buffToSend1[2] = j;

					double buffToSend2[3];
					buffToSend2[0] = matrix[0][i][j];
					buffToSend2[1] = i;
					buffToSend2[2] = j;

					double buffToSend3[3];
					buffToSend3[0] = matrix[0][i][j];
					buffToSend3[1] = i;
					buffToSend3[2] = j;

					double buffToSend4[3];
					buffToSend4[0] = matrix[0][i][j];
					buffToSend4[1] = i;
					buffToSend4[2] = j;

					if((i - 1) >= 0)
					{
						if(mappingMatrix[i - 1][j] != processRank)
							MPI_Send(buffToSend1 , 3, MPI_DOUBLE, mappingMatrix[i - 1][j], 0, MPI_COMM_WORLD);
					}

					if((j - 1) >= 0)
					{
						if(mappingMatrix[i][j - 1] != processRank)
							MPI_Send(buffToSend2 , 3, MPI_DOUBLE, mappingMatrix[i][j - 1], 0, MPI_COMM_WORLD);
					}

					if((i + 1) < realN)
					{
						if(mappingMatrix[i + 1][j] != processRank)
							MPI_Send(buffToSend3 , 3, MPI_DOUBLE, mappingMatrix[i + 1][j], 0, MPI_COMM_WORLD);
					}

					if((j + 1) < realM)
					{
						if(mappingMatrix[i][j + 1] != processRank)
							MPI_Send(buffToSend4 , 3, MPI_DOUBLE, mappingMatrix[i][j + 1], 0, MPI_COMM_WORLD);
					}
				}		
			}
		}
	}
	//}

	for(int k = 0; k < (np - 1); k++)
	{
		int msgToReceiv = 0;

		for (int i = 0; i < realN; i++)
		{
			for (int j = 0; j < realM; j++)
			{
				if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
				{
					if((i - 1) >= 0)
					{
						if(mappingMatrix[i - 1][j] != processRank)
						{
							msgToReceiv++;
						}
					}

					if((j - 1) >= 0)
					{
						if(mappingMatrix[i][j - 1] != processRank)
						{
							msgToReceiv++;
						}
					}

					if((i + 1) < realN)
					{
						if(mappingMatrix[i + 1][j] != processRank)
						{
							msgToReceiv++;
						}
					}

					if((j + 1) < realM)
					{
						if(mappingMatrix[i][j + 1] != processRank)
						{
							msgToReceiv++;
						}
					}
				}
			}
		}

		int msgReceiv = 0;

		while(msgReceiv < msgToReceiv)
		{
			double buffToRecv1[3];
					
			MPI_Status status1;

			MPI_Recv(&buffToRecv1, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
			matrix[k][(int) buffToRecv1[1]][(int) buffToRecv1[2]] = buffToRecv1[0]; 

			msgReceiv++;

		}

		for (int i = 0; i < realN; i++)
		{
			for (int j = 0; j < realM; j++)
			{
				if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
				{
					double val1;
					double val2;
					double val3;
					double val4;

					if((i - 1) >= 0)
					{
						val1 = matrix[k][i - 1][j];
					}
					else
					{
						val1 = 0.0;
					}

					if((i + 1) < realN)
					{
						val2 = matrix[k][i + 1][j];
					}
					else
					{
						val2 = 0.0;
					}

					if((j - 1) >= 0)
					{
						val3 = matrix[k][i][j - 1];
					}
					else
					{
						val3 = 0.0;
					}

					if((j + 1) < realM)
					{
						val4 = matrix[k][i][j + 1];
					}
					else
					{
						val4 = 0.0;
					}

					matrix[k + 1][i][j] = (1.0 - 4.0*td/(h*h)) * matrix[k][i][j] + 
										  (td/(h*h)) * (val1 + val2 + val3 + val4);

				  	if(k + 1 < (np - 1))
					{

						double buffToSend[3];
						buffToSend[0] = matrix[0][i][j];
						buffToSend[1] = i;
						buffToSend[2] = j;

						if((i - 1) >= 0)
						{
							if(mappingMatrix[i - 1][j] != processRank)
								MPI_Send(&buffToSend , 3, MPI_DOUBLE, mappingMatrix[i - 1][j], 0, MPI_COMM_WORLD);
						}

						if((j - 1) >= 0)
						{
							if(mappingMatrix[i][j - 1] != processRank)
								MPI_Send(&buffToSend , 3, MPI_DOUBLE, mappingMatrix[i][j - 1], 0, MPI_COMM_WORLD);
						}

						if((i + 1) < realN)
						{
							if(mappingMatrix[i + 1][j] != processRank)
								MPI_Send(&buffToSend , 3, MPI_DOUBLE, mappingMatrix[i + 1][j], 0, MPI_COMM_WORLD);
						}

						if((j + 1) < realM)
						{
							if(mappingMatrix[i][j + 1] != processRank)
								MPI_Send(&buffToSend , 3, MPI_DOUBLE, mappingMatrix[i][j + 1], 0, MPI_COMM_WORLD);
						}
					}
				}
			}
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);

	if(processRank == 0) //processeur consideré comme le serveur , va recevoir les données des autres processeurs et créer la matrice
	{		
		int msgToReceiv = 0;
		int msgReceiv = 0;

		for (int i = 0; i < realN; i++)
		{
			for (int j = 0; j < realM; j++)
			{
				if(mappingMatrix[i][j] != processRank) // si on est pas sur le "maitre" assigne a cette cellule
				{
					msgToReceiv ++;
				}
			}
			
			while(msgReceiv < msgToReceiv)
			{
				double buffToRecv1[3];
				MPI_Status status1;

				MPI_Recv(buffToRecv1, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
				matrix[np - 1][(int) buffToRecv1[1]][(int) buffToRecv1[2]] = buffToRecv1[0]; 

				msgReceiv++;
			}
		}	
		printf("%s\n", "Parrallele");
		printf("%s\n", "-----------------------------------------------------------------");

		for (int i = 0; i < n; i++)
		{
			printf("XXXXXXXX ");
		}

		printf("\n");

		for(int i = 0; i < realN; i++) 
		{
			
			printf("XXXXXXXX ");

			for(int j = 0; j < realM; j++) 
			{

				printf("%lf ", matrix[np - 1][i][j]);
			}
			
			printf("XXXXXXXX ");

			printf("\n");
		} 

		for (int i = 0; i < n; ++i)
		{
			printf("XXXXXXXX ");
		}

		printf("%s\n", "-----------------------------------------------------------------");
	}
	else
	{
		for (int i = 0; i < realN; i++)
		{
			for (int j = 0; j < realM; j++)
			{
				if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
				{
					
					double buffToSend[3];
					buffToSend[0] = matrix[np - 1][i][j];
					buffToSend[1] = i;
					buffToSend[2] = j;

					MPI_Send(buffToSend , 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
				}
			}
		}
	}
	
	if(processRank == 0)
	{
		double seqmatrix[np][n][m];	

		initMatrix(seqmatrix);

		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
			{
				seqmatrix[0][i][j] = i * (n - i - 1) * j * (m - j - 1);
			} 
		}		

		printf("%s\n", "Matrice après initialisation");
		printMatrix(seqmatrix, 0);

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
					seqmatrix[k + 1][i][j] = (1.0 - 4.0*td/(h*h)) * seqmatrix[k][i][j] + 
										  (td/(h*h)) * (seqmatrix[k][i - 1][j] + seqmatrix[k][i + 1][j] + seqmatrix[k][i][j - 1] + seqmatrix[k][i][j + 1]);
				}
			}
		}

		gettimeofday(&tp, NULL);
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTime = timeEnd - timeStart;

		printf("%s\n", "Sequentielle");
		printMatrix(seqmatrix, np - 1);
		printf("Execution time: %f\n", executionTime);
	}

	MPI_Finalize();
    return 0;
} 