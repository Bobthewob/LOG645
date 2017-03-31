__kernel void VectoraNextNP(__global float* A, float TDH, int M, int N, int NP, __global float* B) {
  
  // Get the index of the current element to be processed
    int i = get_global_id(0);
		
	if (i >= (M*N))
	{   
		B[i] = 0; 
		return;
	}
	if ((i % M) <= 0)
	{
		B[i] = 0; 
		return;
	}
	if ((i / M) <= 0)
	{
		B[i] = 0; 
		return;
	}
	if ((i % M) >= (M - 1))
	{
		B[i] = 0; 
		return;
	}
	if ((i / M) >= (N - 1))
	{
		B[i] = 0; 
		return;
	}

	B[i] = (1.0f - 4.0f * TDH) * A[i] + TDH * 
	(A[i - 1] + A[i + 1] + A[i - M] + A[i + M]);
}