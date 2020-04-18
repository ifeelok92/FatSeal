#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "poly.h"
#include "params.h"
#include "byte.h"
#include "sig.h"
#include "fips202.h"
#include "ntt.h"
#include "rand.h"

#define NTESTS 5000






void compare_p(int32_t *a, int32_t *b, int n){
	for (int i = 0; i < n;i++){
		if(a[i]!=b[i])
		printf("%d:\t%d\t%d\n", i, a[i], b[i]);
	}
}



static void pol_naivemul(uint16_t *c, uint16_t  *a, uint16_t  *b)
{
	int i, j;
	uint16_t r[2 * N];

	for (i = 0; i < 2 * N; i++)
		r[i] = 0;

	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
		{
			r[i + j] += a[i] * b[j] % Q;
			r[i + j] %= Q;
		}

	for (i = N; i < 2 * N - 1; i++) {
		r[i - N] = r[i - N] + Q - r[i];
		r[i - N] %= Q;
	}

	for (i = 0; i < N; i++)
		c[i] = r[i];
}

void benchmarks(){
	clock_t start,finish;
	double t1=0, t2=0, t3=0;
	
	//int32_t arr[5] = { 805306368,2148139008,2148073472,65536,1460142080 };

	int32_t a[N]={0},b[N]={0},c[N]={0},d[N]={1}, i,j=0;

	unsigned char m[256], pk[N_BYTES], sk[N2 + N_BYTES];
	
	unsigned char message[256], j1, rb[N];

	unsigned char sigma[ RLEN + ULEN + L_BYTES];  // ( k, u , a )

	unsigned char rd[48] = { 0 }, bg[48] = {0,1,1};

	rand_init(bg, 48);
	//randombytes(temp, 256);
	randombytes(rd, 48);
	rand_init(rd, 256);
	int count = 0;


	for (i = 0; i < NTESTS; i++) {

		randombytes(message, 256);


		// count keygen time
		start = clock();

		sig_keygen(pk, sk);

		finish = clock();

		t1 = t1+(double)(finish - start) / CLOCKS_PER_SEC;

		// count sign time

		start = clock();

		sig_sign(sk, message, sigma);

		finish = clock();
		t2 = t2 + (double)(finish - start) / CLOCKS_PER_SEC;


		// // count verify time

		start = clock();

		j = sig_verf(pk, sigma, message);
		finish = clock();
		t3 = t3 + (double)(finish - start) / CLOCKS_PER_SEC;

		if (j != 1) {
			count++;
			printf("\n第 %d 次执行\n", i);
			printf("\nwrong\n");
		}
		//else printf("OK\n");  
	}

	 printf("\n%d\t among %d \n", count, NTESTS);
	printf("keygen time  :%f us\n", (t1 * 1000000 / NTESTS));
	printf("sign time  :%f us\n", (t2 * 1000000 / NTESTS));
	printf("verify time  :%f us\n", (t3 * 1000000 / NTESTS));
    printf("\n");
}


int main()
{
	benchmarks();
	return 0;
}