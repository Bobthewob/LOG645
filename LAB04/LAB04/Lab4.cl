__kernel void VectoraNextNP(__global float* A, float TDH, int M, int N, int NP, __global float* B) {
  
  // Get the index of the current element to be processed
    int i = get_global_id(0);
	int skip = 0;
	
	
	for (int k = 0; k <= (NP - 1); k++)
	{
		if (i >= (M*N))
		{   
			B[i] = 0; 
			skip = 1;
		}
		if ((i % M) <= 0)
		{
			B[i] = 0; 
			skip = 1;
		}
		if ((i / M) <= 0)
		{
			B[i] = 0; 
			skip = 1;
		}
		if ((i % M) >= (M - 1))
		{
			B[i] = 0; 
			skip = 1;
		}
		if ((i / M) >= (N - 1))
		{
			B[i] = 0; 
			skip = 1;
		}

		if(skip == 0)
		{
			B[i] = (1 - 4 * TDH) * A[i] + TDH * 
			(A[i - 1] + A[i + 1] + A[i - M] + A[i + M]);
		}

		barrier(CLK_GLOBAL_MEM_FENCE);

		A[i] = B[i];

		barrier(CLK_GLOBAL_MEM_FENCE);
	}
}