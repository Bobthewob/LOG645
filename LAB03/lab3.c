#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys/time.h"
#include "mpi.h"

int err, numberOfProcess, processRank, m, n, np, nbproc;
double timeStart1, timeStart2, timeEnd1, timeEnd2, executionTimeSeq, executionTimePar;
double td, h;
struct timeval tp;

void printMatrix(float matrix[n][m])
{
	printf("%s\n", "-----------------------------------------------------------------");
	for(int i = 0; i < n; i++) 
	{
		for(int j = 0; j < m; j++) 
		{
			printf("%.2f ", matrix[i][j]);
		}
		
		printf("\n");
	} 
	printf("%s\n", "-----------------------------------------------------------------");
}

void initMatrix(float matrix[n][m])
{	
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrix[i][j] = 0.0;
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
	double tdh = td / (h*h);

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);	
	
	//---------------------SEQUENTIELLE----------------------------------------------------------------------
	if(processRank == 0)
	{
		float oldMatrix[n][m];	
		float newMatrix[n][m];

		initMatrix(oldMatrix);
		initMatrix(newMatrix);

		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
			{
				oldMatrix[i][j] = i * (n - i - 1) * j * (m - j - 1);
				newMatrix[i][j] = i * (n - i - 1) * j * (m - j - 1);
			} 
		}			

		printf("%s\n", "Matrice après initialisation");
		printMatrix(newMatrix);

		gettimeofday(&tp, NULL); 
		timeStart1 = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

		for(int k = 0; k <= (np - 1); k++)
		{
			for(int i = 1; i < (n - 1); i++)
			{
				for(int j = 1; j < (m - 1); j++)
				{	
					usleep(5);
					newMatrix[i][j] = (1.0 - 4.0 * tdh) * (oldMatrix[i][j]) + tdh * (oldMatrix[i - 1][j] + oldMatrix[i + 1][j] + oldMatrix[i][j - 1] + oldMatrix[i][j + 1]); 
				}
			}

			for(int i = 1; i < (n - 1); i++)
			{
				for(int j = 1; j < (m - 1); j++)
				{
					oldMatrix[i][j] = newMatrix[i][j];
				}
			}
		}

		gettimeofday(&tp, NULL);
		timeEnd1 = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTimeSeq = timeEnd1 - timeStart1;

		printf("%s\n", "Sequentielle");
		printMatrix(newMatrix);
	}

	MPI_Barrier(MPI_COMM_WORLD);	

	//--------------PARALLELE-----------------------------------------------------------------------------------------------

	if(processRank == 0) //processeur consideré comme le serveur , va recevoir les données des autres processeurs et créer la matrice
	{
		gettimeofday(&tp, NULL); 
	 	timeStart2 = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	}

	float numberOfCells = (realN*realM);
	int nbCellPerProcessor;

	if((numberOfCells / nbproc) == (((int) numberOfCells) / nbproc))
		nbCellPerProcessor = ((int)(numberOfCells / nbproc));
	else
	{
		nbCellPerProcessor = ((int)(numberOfCells / nbproc)) + 1;
	}

	if(nbCellPerProcessor == 0)
		nbCellPerProcessor = 1;

	int mappingMatrix[realN][realM];
	float oldMatrix[realN][realM];	
	float newMatrix[realN][realM];	

	for (int i = 0; i < realN; i++)
	{
		for (int j = 0; j < realM; j++)
		{	
			mappingMatrix[i][j] = -1;
		    oldMatrix[i][j] = 0.0;
            newMatrix[i][j] = 0.0;
		}
	}

	int currentProc = 0;
	int i = 0;
	
	if(realM <= realN)
	{
		while((i + nbCellPerProcessor) <= numberOfCells)
		{	
			for (int j = 0; j < nbCellPerProcessor; ++j)
			{
				mappingMatrix[i / realM][i % realM] = currentProc;
				++i;
			}
			++currentProc;
		}	

		for (int j = 0; j < ((int) numberOfCells - i); ++j)
		{
			mappingMatrix[(i + j) / realM][(i + j) % realM] = currentProc;
		}
	}
	else
	{
		while((i + nbCellPerProcessor) <= numberOfCells)
		{	
			for (int j = 0; j < nbCellPerProcessor; ++j)
			{
				mappingMatrix[i % realN][i / realN] = currentProc;
				++i;
			}
			++currentProc;
		}	

		for (int j = 0; j < ((int) numberOfCells - i); ++j)
		{
			mappingMatrix[(i + j) % realN][(i + j) / realN] = currentProc;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);	

	int ProcessI[nbCellPerProcessor];
	int ProcessJ[nbCellPerProcessor];

	for (int i = 0; i < nbCellPerProcessor; ++i)
	{
		ProcessI[i] = -1;
		ProcessJ[i] = -1;
	}

	int index = 0;
	int activeProcess = 0;

	for (int i = 0; i < realN; i++)
	{
		for (int j = 0; j < realM; j++)
		{
			if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
			{
				ProcessI[index] = i;
				ProcessJ[index] = j;
				index++;
				activeProcess = 1;
			}
		}
	}

	if(activeProcess == 1)
	{

		//k = 0
		for (int index = 0; index < nbCellPerProcessor; index++)
		{
			if(ProcessI[index] != -1)
			{
				int i = ProcessI[index];
				int j = ProcessJ[index];

				usleep(5);
				oldMatrix[i][j] = (i + 1) * (n - i - 2) * (j + 1) * (m - j - 2);
				newMatrix[i][j] = (i + 1) * (n - i - 2) * (j + 1) * (m - j - 2);

				if(np >= 1)
				{
					float buffToSend1[3];
					buffToSend1[0] = newMatrix[i][j];
					buffToSend1[1] = i;
					buffToSend1[2] = j;


					if((i - 1) >= 0)
					{
						if(mappingMatrix[i - 1][j] != processRank)
							MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i - 1][j], 0, MPI_COMM_WORLD);
					}

					if((j - 1) >= 0)
					{
						if(mappingMatrix[i][j - 1] != processRank)
							MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i][j - 1], 0, MPI_COMM_WORLD);
					}

					if((i + 1) < realN)
					{
						if(mappingMatrix[i + 1][j] != processRank)
							MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i + 1][j], 0, MPI_COMM_WORLD);
					}

					if((j + 1) < realM)
					{
						if(mappingMatrix[i][j + 1] != processRank)
							MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i][j + 1], 0, MPI_COMM_WORLD);
					}
				}
			}
		}

		for(int k = 0; k <= (np - 1); k++)
		{
			int msgToReceiv = 0;

			for (int index = 0; index < nbCellPerProcessor; index++)
			{
				if(ProcessI[index] != -1)
				{
					int i = ProcessI[index];
					int j = ProcessJ[index];

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

			int msgReceiv = 0;

			while(msgReceiv < msgToReceiv)
			{
				float buffToRecv1[3];
						
				MPI_Status status1;

				MPI_Recv(&buffToRecv1, 3, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
				oldMatrix[(int) buffToRecv1[1]][(int) buffToRecv1[2]] = buffToRecv1[0]; 

				msgReceiv++;
			}

			MPI_Barrier(MPI_COMM_WORLD);

			for (int index = 0; index < nbCellPerProcessor; index++)
			{
				if(ProcessI[index] != -1)
				{
					int i = ProcessI[index];
					int j = ProcessJ[index];

					float val1;
					float val2;
					float val3;
					float val4;

					if((i - 1) >= 0)
					{
						val1 = oldMatrix[i - 1][j];
					}
					else
					{
						val1 = 0.0;
					}

					if((i + 1) < realN)
					{
						val2 = oldMatrix[i + 1][j];
					}
					else
					{
						val2 = 0.0;
					}

					if((j - 1) >= 0)
					{
						val3 = oldMatrix[i][j - 1];
					}
					else
					{
						val3 = 0.0;
					}

					if((j + 1) < realM)
					{
						val4 = oldMatrix[i][j + 1];
					}
					else
					{
						val4 = 0.0;
					}

					usleep(5);
					newMatrix[i][j] = (1.0 - 4.0 * tdh) * oldMatrix[i][j] + 
										  tdh * (val1 + val2 + val3 + val4);

				  	if(k + 1 <= (np - 1))
					{
						float buffToSend1[3];
						buffToSend1[0] = newMatrix[i][j];
						buffToSend1[1] = i;
						buffToSend1[2] = j;

						if((i - 1) >= 0)
						{
							if(mappingMatrix[i - 1][j] != processRank)
								MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i - 1][j], 0, MPI_COMM_WORLD);
						}

						if((j - 1) >= 0)
						{
							if(mappingMatrix[i][j - 1] != processRank)
								MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i][j - 1], 0, MPI_COMM_WORLD);
						}

						if((i + 1) < realN)
						{
							if(mappingMatrix[i + 1][j] != processRank)
								MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i + 1][j], 0, MPI_COMM_WORLD);
						}

						if((j + 1) < realM)
						{
							if(mappingMatrix[i][j + 1] != processRank)
								MPI_Send(&buffToSend1 , 3, MPI_FLOAT, mappingMatrix[i][j + 1], 0, MPI_COMM_WORLD);
						}
					}
				}
			}

			for(int i = 0; i < realN; i++)
			{
				for(int j = 0; j < realM; j++)
				{
					oldMatrix[i][j] = newMatrix[i][j];
				}
			}
		}
	}
	else
	{
		for(int k = 0; k <= (np - 1); k++)
		{
			MPI_Barrier(MPI_COMM_WORLD);
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
					msgToReceiv++;
				}
			}
		}

		while(msgReceiv < msgToReceiv)
		{
			float buffToRecv1[3];
			MPI_Status status1;

			MPI_Recv(buffToRecv1, 3, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
			newMatrix[(int) buffToRecv1[1]][(int) buffToRecv1[2]] = buffToRecv1[0]; 

			msgReceiv++;
		}

		gettimeofday(&tp, NULL);
		timeEnd2 = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		executionTimePar = timeEnd2 - timeStart2;

		printf("%s\n", "Parallele");
		printf("%s\n", "-----------------------------------------------------------------");

		for (int i = 0; i < m; i++)
		{
			printf("0.00 ");
		}

		printf("\n");

		for(int i = 0; i < realN; i++) 
		{
			
			printf("0.00 ");

			for(int j = 0; j < realM; j++) 
			{

				printf("%.2f ", newMatrix[i][j]);
			}
			
			printf("0.00 ");

			printf("\n");
		} 

		for (int i = 0; i < m; ++i)
		{
			printf("0.00 ");
		}

		printf("\n");
		printf("%s\n", "-----------------------------------------------------------------");
		printf("Execution time Sequentielle: %f\n", executionTimeSeq);
		printf("Execution time parallele : %f\n", executionTimePar);
		printf("Acceleration : %f\n", executionTimeSeq / executionTimePar);
	}
	else if(activeProcess == 1)
	{
		for (int i = 0; i < realN; i++)
		{
			for (int j = 0; j < realM; j++)
			{
				if(mappingMatrix[i][j] == processRank) // si on est sur le processeur assigne a cette cellule
				{
					
					float buffToSend[3];
					buffToSend[0] = newMatrix[i][j];
					buffToSend[1] = i;
					buffToSend[2] = j;

					MPI_Send(buffToSend , 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
				}
			}
		}
	}
	
	MPI_Finalize();
    return 0;
} 