#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <windows.h>

// *********************************************************************
// Ce projet est basé sur l'exemple oclVectorAdd fait par NVIDIA recommandé
// par le chargé de laboratoire.
// *********************************************************************
void *srcA, *srcB;        // Host buffers for OpenCL test
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device
cl_program cpProgram;           // OpenCL program
cl_kernel ckKernel;             // OpenCL kernel
cl_mem buffA;               // OpenCL device source buffer A
cl_mem buffB;                // OpenCL device destination buffer 
size_t szGlobalWorkSize;        // 1D var for Total # of work items
size_t szLocalWorkSize;		    // 1D var for # of work items in the work group	
size_t szKernelLength;			// Byte size of kernel code
cl_int ciErr1;			// Error code var
char* cSourceCL = NULL;         // Buffer to hold source for compilation
int m, n, np;
double executionTimeSeq, executionTimePar, td, h;
double PCFreq = 0.0;
__int64 CounterStart = 0;

//inspire de http://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
void StartCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

//Code tiré de l'annexe fournie dans l'énoncé
char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t*szFinalLength)
{
	FILE* pFileStream = NULL;
	size_t szSourceLength;

	// open the OpenCL source code file
	if (fopen_s(&pFileStream, cFilename, "rb") != 0)
	{
		return NULL;
	}

	size_t szPreambleLength = strlen(cPreamble);

	// get the length of the source code
	fseek(pFileStream, 0, SEEK_END);
	szSourceLength = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);

	// allocate a buffer for the source code string and read it in
	char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
	memcpy(cSourceString, cPreamble, szPreambleLength);

	if (fread((cSourceString)+szPreambleLength, szSourceLength, 1, pFileStream) != 1)
	{
		fclose(pFileStream);
		free(cSourceString);
		return 0;
	}

	// close the file and return the total length of the combined (preamble + source) string 
	fclose(pFileStream);

	if (szFinalLength != 0)
	{
		*szFinalLength = szSourceLength + szPreambleLength;
	}

	cSourceString[szSourceLength + szPreambleLength] = '\0';
	return cSourceString;
}

int main(int argc, char *argv[])
{

	if (argc != 6)
	{
		printf("Nombre de parametres invalides");
		getchar();
		return 0;
	}

	n = atoi(argv[1]); //nombre de lignes
	m = atoi(argv[2]); //nombre de colonnes
	np = atoi(argv[3]); //nombre de pas de temps
	td = atof(argv[4]); //le temps discretise
	h = atof(argv[5]); //la taille d'un cote d'une subdivision
	float tdh = (float)td / (float)(h*h);
	szLocalWorkSize = 256;
	szGlobalWorkSize = m * n; // Taille de la matrice

							  //---------------------SEQUENTIELLE----------------------------------------------------------------------
	void *oldMatrix, *newMatrix;

	//Allocation de la mémoire
	oldMatrix = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
	newMatrix = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);

	// Initialisation de la matrice
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			((float*)oldMatrix)[i + (j * n)] = (float)i * (n - i - 1) * j * (m - j - 1);
			((float*)newMatrix)[i + (j * n)] = (float)i * (n - i - 1) * j * (m - j - 1);
		}
	}

	// Affichage de la matrice après initialisation
	printf("%s\n", "Matrice apres initialisation");
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", ((float*)oldMatrix)[i + (j * n)]);
		}
		printf("\r\n");
	}

	StartCounter();

	for (int k = 0; k <= (np - 1); k++)
	{
		for (int i = 0; i < m * n; i++)
		{
			if (i >= (m*n))
			{
				((float*)newMatrix)[i] = 0;
				continue;
			}
			if ((i % n) <= 0)
			{
				((float*)newMatrix)[i] = 0;
				continue;
			}
			if ((i / n) <= 0)
			{
				((float*)newMatrix)[i] = 0;
				continue;
			}
			if ((i % n) >= (n - 1))
			{
				((float*)newMatrix)[i] = 0;
				continue;
			}
			if ((i / n) >= (m - 1))
			{
				((float*)newMatrix)[i] = 0;
				continue;
			}

			((float*)newMatrix)[i] = (1.0f - 4.0f * tdh) * ((float*)oldMatrix)[i] + tdh *
				(((float*)oldMatrix)[i - 1] + ((float*)oldMatrix)[i + 1] + ((float*)oldMatrix)[i - n] + ((float*)oldMatrix)[i + n]);

		}
		memmove(oldMatrix, newMatrix, sizeof(float) * szGlobalWorkSize);
	}

	executionTimeSeq = GetCounter();

	// Affichage de la matrice après exécution séquentielle
	printf("%s\n", "Sequentielle");
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", ((float*)newMatrix)[i + (j * n)]);
		}
		printf("\r\n");
	}
	printf("\r\n");

	//-------------------------------------------PARALLELE-----------------------------------------------------------------
	// Allocation de la mémoire
	srcA = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
	srcB = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			((float*)srcA)[i + (j * n)] = (float)i * (n - i - 1) * j * (m - j - 1);
		}
	}

	// Obtention de la plateforme OpenCL
	ciErr1 = clGetPlatformIDs(1, &cpPlatform, NULL);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Erreur dans clGetPlatformID, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Obtention des devices (GPUs)
	ciErr1 = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clGetDeviceIDs, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Création du contexte
	cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateContext, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Création de la commande
	cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Création des bufffers dans le GPU
	buffA = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);
	buffB = clCreateBuffer(cxGPUContext, CL_MEM_WRITE_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	cSourceCL = oclLoadProgSource("Lab4.cl", "", &szKernelLength);

	// Création du programme
	cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateProgramWithSource, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	ciErr1 = clBuildProgram(cpProgram, 0, NULL, NULL, NULL, NULL);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clBuildProgram, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Création du Kernel
	ckKernel = clCreateKernel(cpProgram, "VectoraNextNP", &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Définition des arguments passés au GPU
	ciErr1 = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&buffA);
	ciErr1 |= clSetKernelArg(ckKernel, 1, sizeof(cl_float), (void*)&tdh);
	ciErr1 |= clSetKernelArg(ckKernel, 2, sizeof(cl_int), (void*)&n);
	ciErr1 |= clSetKernelArg(ckKernel, 3, sizeof(cl_int), (void*)&m);
	ciErr1 |= clSetKernelArg(ckKernel, 4, sizeof(cl_int), (void*)&np);
	ciErr1 |= clSetKernelArg(ckKernel, 5, sizeof(cl_mem), (void*)&buffB);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clSetKernelArg, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	StartCounter();

	for (int k = 0; k <= (np - 1); k++)
	{
		// Envoie asynchrone des données au GPU
		ciErr1 = clEnqueueWriteBuffer(cqCommandQueue, buffA, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcA, 0, NULL, NULL);
		ciErr1 |= clEnqueueWriteBuffer(cqCommandQueue, buffB, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);
		if (ciErr1 != CL_SUCCESS)
		{
			printf("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		}

		// Appel du Kernel
		ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &szGlobalWorkSize, NULL, 0, NULL, NULL);
		if (ciErr1 != CL_SUCCESS)
		{
			int maxGlobSize = CL_DEVICE_MAX_WORK_GROUP_SIZE;
			printf("Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
			getchar();
		}

		// Lecture synchrone du buffer
		ciErr1 = clEnqueueReadBuffer(cqCommandQueue, buffB, CL_TRUE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);

		srcA = srcB;
	}

	executionTimePar = GetCounter();

	// Affichage de la matrice finale après exécution parallèle
	printf("Parallele \r\n");
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", ((float*)srcB)[i + (j * n)]);
		}
		printf("\r\n");
	}

	printf("temps sequentiel : %.8f \r\n", executionTimeSeq / 1000);
	printf("temps parallele : %.8f \r\n", executionTimePar / 1000);
	printf("acceleration : %.8f \r\n", executionTimeSeq / executionTimePar);

	getchar();

	return 0;
}