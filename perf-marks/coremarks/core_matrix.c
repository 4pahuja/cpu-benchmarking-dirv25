/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/

#include "coremark.h"
/*
Topic: Description
	Matrix manipulation benchmark

	This very simple algorithm forms the basis of many more complex algorithms.

	The tight inner loop is the focus of many optimizations (compiler as well as hardware based)
	and is thus relevant for embedded processing.

	The total available data space will be divided to 3 parts:
	NxN Matrix A - initialized with small values (upper 3/4 of the bits all zero).
	NxN Matrix B - initialized with medium values (upper half of the bits all zero).
	NxN Matrix C - used for the result.

	The actual values for A and B must be derived based on input that is not available at compile time.
*/
ee_s16 matrix_test(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val);
ee_s16 matrix_sum(ee_u32 N, MATRES *C, MATDAT clipval);
void matrix_mul_const(ee_u32 N, MATRES *C, MATDAT *A, MATDAT val);
void matrix_mul_vect(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix_bitextract(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_add_const(ee_u32 N, MATDAT *A, MATDAT val);

#define matrix_test_next(x) (x+1)
#define matrix_clip(x,y) ((y) ? (x) & 0x0ff : (x) & 0x0ffff)
#define matrix_big(x) (0xf000 | (x))
#define bit_extract(x,from,to) (((x)>>(from)) & (~(0xffffffff << (to))))

#if CORE_DEBUG
void printmat(MATDAT *A, ee_u32 N, char *name) {
	ee_u32 i,j;
	ee_printf("Matrix %s [%dx%d]:\n",name,N,N);
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			if (j!=0)
				ee_printf(",");
			ee_printf("%d",A[i*N+j]);
		}
		ee_printf("\n");
	}
}
void printmatC(MATRES *C, ee_u32 N, char *name) {
	ee_u32 i,j;
	ee_printf("Matrix %s [%dx%d]:\n",name,N,N);
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			if (j!=0)
				ee_printf(",");
			ee_printf("%d",C[i*N+j]);
		}
		ee_printf("\n");
	}
}
#endif
/* Function: core_bench_matrix
	Benchmark function

	Iterate <matrix_test> N times,
	changing the matrix values slightly by a constant amount each time.
*/
ee_u16 core_bench_matrix(mat_params *p, ee_s16 seed, ee_u16 crc) {
	ee_u32 N=p->N;
	MATRES *C=p->C;
	MATDAT *A=p->A;
	MATDAT *B=p->B;
	MATDAT val=(MATDAT)seed;

	crc=crc16(matrix_test(N,C,A,B,val),crc);

	return crc;
}

/* Function: matrix_test
	Perform matrix manipulation.

	Parameters:
	N - Dimensions of the matrix.
	C - memory for result matrix.
	A - input matrix
	B - operator matrix (not changed during operations)

	Returns:
	A CRC value that captures all results calculated in the function.
	In particular, crc of the value calculated on the result matrix
	after each step by <matrix_sum>.

	Operation:

	1 - Add a constant value to all elements of a matrix.
	2 - Multiply a matrix by a constant.
	3 - Multiply a matrix by a vector.
	4 - Multiply a matrix by a matrix.
	5 - Add a constant value to all elements of a matrix.

	After the last step, matrix A is back to original contents.
*/
ee_s16 matrix_test(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val) {
	ee_u16 crc=0;
	MATDAT clipval=matrix_big(val);

	matrix_add_const(N,A,val); /* make sure data changes  */
#if CORE_DEBUG
	printmat(A,N,"matrix_add_const");
#endif
	matrix_mul_const(N,C,A,val);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_const");
#endif
	matrix_mul_vect(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_vect");
#endif
	matrix_mul_matrix(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_matrix");
#endif
	matrix_mul_matrix_bitextract(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_matrix_bitextract");
#endif

	matrix_add_const(N,A,-val); /* return matrix to initial value */
	return crc;
}

/* Function : matrix_init
	Initialize the memory block for matrix benchmarking.

	Parameters:
	blksize - Size of memory to be initialized.
	memblk - Pointer to memory block.
	seed - Actual values chosen depend on the seed parameter.
	p - pointers to <mat_params> containing initialized matrixes.

	Returns:
	Matrix dimensions.

	Note:
	The seed parameter MUST be supplied from a source that cannot be determined at compile time
*/
ee_u32 core_init_matrix(ee_u32 blksize, void *memblk, ee_s32 seed, mat_params *p) {
	ee_u32 N=0;
	MATDAT *A;
	MATDAT *B;
	ee_s32 order=1;
	MATDAT val;
	ee_u32 i=0,j=0;
	if (seed==0)
		seed=1;
	while (j<blksize) {
		i++;
		j=i*i*2*4;
	}
	N=i-1;
	A=(MATDAT *)align_mem(memblk);
	B=A+N*N;
	//j = N*N;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			seed = ( ( order * seed ) % 65536 );
			val = (seed + order);
			val=matrix_clip(val,0);
			B[i*N+j] = val;
			val =  (val + order);
			val=matrix_clip(val,1);
			A[i*N+j] = val;
			order++;
		}
	}

	p->A=A;
	p->B=B;
	p->C=(MATRES *)align_mem(B+N*N);
	p->N=N;
#if CORE_DEBUG
	printmat(A,N,"A");
	printmat(B,N,"B");
#endif
	return N;
}

/* Function: matrix_sum
	Calculate a function that depends on the values of elements in the matrix.

	For each element, accumulate into a temporary variable.

	As long as this value is under the parameter clipval,
	add 1 to the result if the element is bigger then the previous.

	Otherwise, reset the accumulator and add 10 to the result.
*/
ee_s16 matrix_sum(ee_u32 N, MATRES *C, MATDAT clipval) {
	MATRES tmp=0,prev=0,cur=0;
	ee_s16 ret=0;
	ee_u32 i, temp = N*N;
	for (i=0; i<temp/*N*/; i++) {
		//for (j=0; j<N; j++) {
			cur=C[i];
			tmp+=cur;
			if (tmp>clipval) {
				ret+=10;
				tmp=0;
			} else {
				ret += (cur>prev) ? 1 : 0;
			}
			prev=cur;
		//}
	}
	return ret;
}

/* Function: matrix_mul_const
	Multiply a matrix by a constant.
	This could be used as a scaler for instance.
*/
void matrix_mul_const(ee_u32 N, MATRES *C, MATDAT *A, MATDAT val) {
	ee_u32 i,j = N*N;
	for (i=0; i<j; i++) {
		//for (j=0; j<N; j++) {
			C[i]=(MATRES)A[i] * (MATRES)val;
		//}
	}
}

/* Function: matrix_add_const
	Add a constant value to all elements of a matrix.
*/
void matrix_add_const(ee_u32 N, MATDAT *A, MATDAT val) {
	ee_u32 i,j=N*N;
	for (i=0; i<j; i++) {
		//for (j=0; j<N; j++) {
			A[i] += val;
		//}
	}
}

/* Function: matrix_mul_vect
	Multiply a matrix by a vector.
	This is common in many simple filters (e.g. fir where a vector of coefficients is applied to the matrix.)
*/
void matrix_mul_vect(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u32 i,j;

	for (i=0; i<N; i++) {
		C[i]=0;
		for (j=0; j<N;j++) {
			C[i]+=(MATRES)A[i*N+j] * (MATRES)B[j];
		}
	}/*
	for (i=0; i<temp; i++) {
		if(i%N==0){

			C[i/N]=0;
		}
		//for (j=0; j<N; j++) {
			C[i/N]+=(MATRES)A[i] * (MATRES)B[i%N];
		//}
	}*/
}

/* Function: matrix_mul_matrix
	Multiply a matrix by a matrix.
	Basic code is used in many algorithms, mostly with minor changes such as scaling.
*/
void matrix_mul_matrix(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u32 i,j,k;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i*N+j]=0;
			for(k=0;k<N;k++)
			{
				C[i*N+j]+=(MATRES)A[i*N+k] * (MATRES)B[k*N+j];
			}
		}
	}
	/*Less efficient
	for (i=0; i<j; i++) {
			C[i]=0;
			for(k=0;k<N;k++)
			{
				C[i]+=(MATRES)A[i-i%N+k] * (MATRES)B[k*N+i%N];
			}
		//}
	}*/
}

/* Function: matrix_mul_matrix_bitextract
	Multiply a matrix by a matrix, and extract some bits from the result.
	Basic code is used in many algorithms, mostly with minor changes such as scaling.
*/
void matrix_mul_matrix_bitextract(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {

/*asm("mv	a6,a0\n"
    "mv	t6,a1\n"
    "mv	t3,a2\n"
    "mv	t1,a3\n"
    "mv	a7,a0\n"
    "li	t5,0\n"
    "li	t0,0\n"
    "beqz a0,matrix_mul_matrix_bitextract+0x90\n"
    "li	t4,0\n"
    "addw a1,t4,t5\n"
    "sll a5,a1,0x20\n"
    "srl a1,a5,0x1e\n"
    "add a1,a1,t6\n"
    "sw	zero,0(a1)\n"
    "mv	a2,t4\n"
    "mv	a3,t5\n"
    "nop\n"
    "sll a4,a3,0x20\n"
    "sll a0,a2,0x20\n"
    "srl a5,a4,0x1f\n"
    "srl a4,a0,0x1f\n"
    "add a5,a5,t3\n"
    "add a4,a4,t1\n"
    "lh	a4,0(a4)\n"
    "lh	a5,0(a5)\n"
    "lw	a0,0(a1)\n"
    "mulw	a5,a5,a4\n"
    "sraw	a4,a5,0x2\n"
    "sraw	a5,a5,0x5\n"
    " 	and	a4,a4,15\n"
    "and	a5,a5,127\n"
    "mulw	a5,a4,a5\n"
    " 	addw	a5,a5,a0\n"
    " 	sw	a5,0(a1)\n"
    " 	nop\n"
    " 	addw	a3,a3,1\n"
    "addw	a2,a6,a2\n"
    "bne	a7,a3,matrix_mul_matrix_bitextract+0x28\n"
    "addw	a5,t4,1\n"
    "beq	a6,a5,matrix_mul_matrix_bitextract+0x7a\n"
    " 	mv	t4,a5\n"
    " 	j	matrix_mul_matrix_bitextract+0x12\n"
    "addw	a5,t0,1\n"
    "addw	t5,a6,t5\n"
    "addw	a7,a6,a7\n"
    "beq	t0,t4,matrix_mul_matrix_bitextract+0x8e\n"
    " 	mv	t0,a5\n"
    " 	j	matrix_mul_matrix_bitextract+0x10\n"
    " 	ret\n"
    " 	ret\n"

);*/
	ee_u32 i,j=N*N;
	for (i=0; i<j; i++) {
		//for (j=0; j<N; j++) {
			C[i]=0;
			//for(k=0;k<1;k++)//SKIP K
			//{
				MATRES tmp=/*(MATRES)A[i-i%N] * */(MATRES)B[i];
				C[i]=bit_extract(tmp,0,4)/**bit_extract(tmp,5,7)*/;
				//MATRES tmp2;
				//if(k!=(N-1)) tmp2=(MATRES)A[i*N+k+1] * (MATRES)B[(k+1)*N+j];
				//C[i*N+j]+=bit_extract(tmp2,2,4)*bit_extract(tmp2,5,7);

			//}
			//printf("%d \n",C[i*N+j]);

		//}
	}
}
