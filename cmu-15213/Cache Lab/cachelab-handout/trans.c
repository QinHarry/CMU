/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int tmp[12] = {0}; // 12 local int to be utilized

	if(M == 32 && N == 32)  // in this case, the transpose block is set with 8x8
	{
	for (tmp[8] = 0; tmp[8] < N; tmp[8]+=8) {
		    for (tmp[9] = 0; tmp[9] < M; tmp[9]++)
		    {
		            tmp[0] = A[tmp[8]][tmp[9]];   // read 8 numbers per time, miss once
		            tmp[1] = A[tmp[8]+1][tmp[9]];
		            tmp[2] = A[tmp[8]+2][tmp[9]];
		            tmp[3] = A[tmp[8]+3][tmp[9]];
		            tmp[4] = A[tmp[8]+4][tmp[9]];
		            tmp[5] = A[tmp[8]+5][tmp[9]];
		            tmp[6] = A[tmp[8]+6][tmp[9]];
		            tmp[7] = A[tmp[8]+7][tmp[9]];

		            B[tmp[9]][tmp[8]] = tmp[0];   // as the set is 8 integer and 32 sets, the matrix A have 4 sets per line.
		            B[tmp[9]][tmp[8]+1] = tmp[1]; // so the miss of this case is 8 for the 1st time
		            B[tmp[9]][tmp[8]+2] = tmp[2];
		            B[tmp[9]][tmp[8]+3] = tmp[3];
		            B[tmp[9]][tmp[8]+4] = tmp[4];
		            B[tmp[9]][tmp[8]+5] = tmp[5];
		            B[tmp[9]][tmp[8]+6] = tmp[6];
		            B[tmp[9]][tmp[8]+7] = tmp[7];
		    }
	    }
	}
	/*
     * The matrix 64x64 is very difficult as there is one key problem:
     * the matrix is 8 sets per line and this will cause the 1st line and the 5th line in the same block
     * will cause miss, so the maximum operand range of cache is 4 vertical lines of matrix. 
     * But the cache read 8 integers per block. So due to this situation, some tricks need to be considered.
     * The details are described below:
     * 1 process the matrix by 8x8 unit block and in each unit do:
     * 2 divide the unit to 4 4x4 block to A11(left up 4x4 block), A22(right up 4x4 block)
     *                                     A33(left down 4x4 block), A44(right down 4x4 block)
     *   the matrix B has the same division name                                  
     * 3 read the up 4 lines of A11 and A22 to B11 and B22(as the A22 should go to B33, but this will cause miss, so 
     *   store A22 to B22 for temporary) 
     * 4 read B22 per 4 integers(this moment B22 still in the cache) to the local integers.
     *   read A33 to B22.
     * 5 after above step, store local integers and A44 to the B44
     *
     */
        else if(M == 64 && N == 64)
        {   
        for (tmp[0] = 0; tmp[0] < 64; tmp[0] += 8) // x
        {
            for (tmp[1] = 0; tmp[1] < 64; tmp[1] += 8) // y
            {
                // move upper 4 lines A11 and A22 to the B11 and B22 matrix upper 4 lines
                for(tmp[2] = tmp[1]; tmp[2] < tmp[1]+4; tmp[2]++) // y
                {
                    tmp[3] = A[tmp[2]][tmp[0]];
                    tmp[4] = A[tmp[2]][tmp[0]+1];
                    tmp[5] = A[tmp[2]][tmp[0]+2];
                    tmp[6] = A[tmp[2]][tmp[0]+3];
                    tmp[7] = A[tmp[2]][tmp[0]+4];
                    tmp[8] = A[tmp[2]][tmp[0]+5];
                    tmp[9] = A[tmp[2]][tmp[0]+6];
                    tmp[10] = A[tmp[2]][tmp[0]+7];
                    B[tmp[0]][tmp[2]] = tmp[3];
                    B[tmp[0]+1][tmp[2]] = tmp[4];
                    B[tmp[0]+2][tmp[2]] = tmp[5];
                    B[tmp[0]+3][tmp[2]] = tmp[6];
                    B[tmp[0]][tmp[2]+4] = tmp[7];
                    B[tmp[0]+1][tmp[2]+4] = tmp[8];
                    B[tmp[0]+2][tmp[2]+4] = tmp[9];
                    B[tmp[0]+3][tmp[2]+4] = tmp[10];
                }
                // fill lower part of Matrix B  
                for(tmp[3] = tmp[0]+4; tmp[3]<tmp[0]+8;tmp[3]++)
                {
                    // store B22 to local integers
                    tmp[4] = B[tmp[3]-4][tmp[1]+4];
                    tmp[5] = B[tmp[3]-4][tmp[1]+5];
                    tmp[6] = B[tmp[3]-4][tmp[1]+6];
                    tmp[7] = B[tmp[3]-4][tmp[1]+7];
                    // store A33 to B22
                    B[tmp[3]-4][tmp[1]+4] = A[tmp[1]+4][tmp[3]-4];
                                        B[tmp[3]-4][tmp[1]+5] = A[tmp[1]+5][tmp[3]-4];
                    B[tmp[3]-4][tmp[1]+6] = A[tmp[1]+6][tmp[3]-4];
                    B[tmp[3]-4][tmp[1]+7] = A[tmp[1]+7][tmp[3]-4];
                    // store local integers to B33
                    B[tmp[3]][tmp[1]] = tmp[4];
                    B[tmp[3]][tmp[1]+1] = tmp[5];
                    B[tmp[3]][tmp[1]+2] = tmp[6];
                    B[tmp[3]][tmp[1]+3] = tmp[7];
                    // load A44 to B44
                    B[tmp[3]][tmp[1]+4] = A[tmp[1]+4][tmp[3]];
                    B[tmp[3]][tmp[1]+5] = A[tmp[1]+5][tmp[3]];
                    B[tmp[3]][tmp[1]+6] = A[tmp[1]+6][tmp[3]];
                    B[tmp[3]][tmp[1]+7] = A[tmp[1]+7][tmp[3]];
                }
            }
        }
    }
    /*
     * This is much easier than the above as the standard is much lower
     * So I just test this proble with different size and store the disgonal element in 
     * the local integer than store the local integer to the destination matrix
     * I can also do the optimization as above, but I think below optimization is enough.
     */                 
        else if(M == 61 && N == 67)
    {
        for(tmp[0] = 0; tmp[0] < 72; tmp[0]+= 32)
            {       
                for(tmp[1] = 0; tmp[1] < 64; tmp[1]+= 16)
                    {      
                            tmp[4] = (tmp[0]+32) > 64? 67:tmp[0]+32;
                            for(tmp[2] = tmp[0]; tmp[2] < tmp[4]; tmp[2]++)
                            {
                                    tmp[5] = (tmp[1]+16)>61?61:tmp[1]+16;     
                                    for(tmp[3] = tmp[1]; tmp[3] < tmp[5]; tmp[3]++)
                                    {
                        if(tmp[3] == tmp[2])
                        {
                            tmp[6] = A[tmp[2]][tmp[3]];
                            tmp[7] = 1;
                        }   
                        else
                                                B[tmp[3]][tmp[2]] = A[tmp[2]][tmp[3]];
                                    }
                    if(tmp[7])
                                    {
                        B[tmp[2]][tmp[2]] = tmp[6];
                        tmp[7] = 0;
                                    }
                }
                    }
            }
    } 
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

