#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <CL/cl.h>
//#include <winbase.h>

//using namespace std

#define MAX_SOURCE_SIZE (0x100000)

char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t*szFinalLength)
{

	// locals
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

int roundUp(int numToRound, int multiple)
{
	if (multiple == 0)
	{
		return numToRound;
	}

	int roundDown = ((int)(numToRound) / multiple) * multiple;
	int roundUp = roundDown + multiple;
	int roundCalc = roundUp;
	return (roundCalc);
}

// Name of the file with the source code for the computation kernel
// *********************************************************************
const char* cSourceFile = "Lab4.cl";

// Host buffers for demo
// *********************************************************************
void *srcA, *srcB, *srcTDH, *srcM, *srcN;        // Host buffers for OpenCL test
//float srcTDH;
//int srcM, srcM;
								// OpenCL Vars
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device
cl_program cpProgram;           // OpenCL program
cl_kernel ckKernel;             // OpenCL kernel
cl_mem buffA;               // OpenCL device source buffer A
cl_mem buffTDH;               // OpenCL device source buffer B 
cl_mem buffM;               // OpenCL device source buffer A
cl_mem buffN;               // OpenCL device source buffer B 
cl_mem buffB;                // OpenCL device destination buffer 
size_t szGlobalWorkSize;        // 1D var for Total # of work items
size_t szLocalWorkSize;		    // 1D var for # of work items in the work group	
size_t szParmDataBytes;			// Byte size of context information
size_t szKernelLength;			// Byte size of kernel code
cl_int ciErr1, ciErr2;			// Error code var
char* cPathAndName = NULL;      // var for full paths to data, src, etc.
char* cSourceCL = NULL;         // Buffer to hold source for compilation
const char* cExecutableName = NULL;


int err, m, n, np, nbproc;
double timeStart1, timeStart2, timeEnd1, timeEnd2, executionTimeSeq, executionTimePar;
double td, h;
unsigned long tp;


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
	float tdh = td / (h*h);

	#define M  m;
	#define N  n;

	//---------------------SEQUENTIELLE----------------------------------------------------------------------

	float oldMatrix[300][300];
	float newMatrix[300][300];

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			oldMatrix[i][j] = i * (n - i - 1) * j * (m - j - 1);
			newMatrix[i][j] = i * (n - i - 1) * j * (m - j - 1);
		}
	}

	printf("%s\n", "Matrice apr�s initialisation");

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", newMatrix[i][j]);
		}
		printf("\r\n");
	}

	//tp = GetTickCount();
	timeStart1 = tp / 1e6;

	for (int k = 0; k <= (np - 1); k++)
	{
		for (int i = 1; i < (n - 1); i++)
		{
			for (int j = 1; j < (m - 1); j++)
			{
				//usleep(5);
				newMatrix[i][j] = (1.0 - 4.0 * tdh) * (oldMatrix[i][j]) + tdh * (oldMatrix[i - 1][j] + oldMatrix[i + 1][j] + oldMatrix[i][j - 1] + oldMatrix[i][j + 1]);
			}
		}

		for (int i = 1; i < (n - 1); i++)
		{
			for (int j = 1; j < (m - 1); j++)
			{
				oldMatrix[i][j] = newMatrix[i][j];
			}
		}
	}

	//tp = GetTickCount();
	timeEnd1 = tp / 1e6;
	executionTimeSeq = timeEnd1 - timeStart1;

	printf("%s\n", "Sequentielle");

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", newMatrix[i][j]);
		}
		printf("\r\n");
	}
	printf("\r\n");

	//-------------------------------------------PARALLELE-----------------------------------------------------------------

	szLocalWorkSize = 256;
	szGlobalWorkSize = m * n;  // rounded up to the nearest multiple of the LocalWorkSize

							   // Allocate and initialize host arrays 
	srcA = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
	srcB = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			((float*)srcA)[i + (j * n)] = i * (n - i - 1) * j * (m - j - 1);
		}
	}

	//Get an OpenCL platform
	ciErr1 = clGetPlatformIDs(1, &cpPlatform, NULL);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Erreur dans clGetPlatformID, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	//Get the devices
	ciErr1 = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clGetDeviceIDs, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	//Create the context
	cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &ciErr1);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateContext, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Create a command-queue
	cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &ciErr1);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Allocate the OpenCL buffer memory objects for source and result on the device GMEM
	buffA = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);
	buffB = clCreateBuffer(cxGPUContext, CL_MEM_WRITE_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	cSourceCL = oclLoadProgSource("Lab4.cl", "", &szKernelLength);

	// Create the program
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

		if (ciErr1 == CL_BUILD_PROGRAM_FAILURE) {
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char *log = (char *)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			printf("%s\n", log);
		}

		getchar();
	}

	// Create the kernel
	ckKernel = clCreateKernel(cpProgram, "VectoraNextNP", &ciErr1);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Set the Argument values
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

	// Asynchronous write of data to GPU device
	ciErr1 = clEnqueueWriteBuffer(cqCommandQueue, buffA, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcA, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(cqCommandQueue, buffB, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
	}

	// Launch kernel
	ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &szGlobalWorkSize, NULL, 0, NULL, NULL);

	if (ciErr1 != CL_SUCCESS)
	{
		int maxGlobSize = CL_DEVICE_MAX_WORK_GROUP_SIZE;
		printf("Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(cqCommandQueue, buffB, CL_TRUE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);

	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		getchar();
	}

	printf("Parallele \r\n");

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%.2f ", ((float*)srcB)[i + (j * n)]);
		}
		printf("\r\n");
	}

	float sup = ((float*)srcB)[1 + n];
	float sup1 = newMatrix[1][1];

	printf("%.2f \r\n", sup);
	printf("%.2f \r\n", sup1);

	getchar();

	return 0;
}