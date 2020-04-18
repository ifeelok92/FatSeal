#include "poly.h"



void print_poly(int32_t *f){
	printf("\n{");
	for (int i = 0; i < N;i++){
		printf("%d,", f[i]);
	}
	printf("}\n");

}


/* Generate a random polynomial
 * output f with a determined number of coefficients equal to 0, -1, and 1(w.r.t N-2*d-1,d,d+1) 
 * */
void trinary_poly_gen(int32_t  *f) {

	unsigned char short_seed[32];
	unsigned char long_seed[1176]; // 7*168
	int seed_len = 1176;
	
	int i;
	int pos;
	int  t = 0;
	int32_t keep;
	int d = 256;
	int32_t upper = (1 << 16) - 1;

	keccak_state state;
	rand_byts(32, short_seed);

	uint16_t nonce = (short_seed[0] << 8) | short_seed[1];
	shake128_stream_init(&state, short_seed, nonce);
	shake128_squeezeblocks(long_seed, 7, &state);
	memset(f, 0, sizeof(int32_t) * 1024);

	for (i = 0; i < d + 1; )
	{
		keep = (long_seed[t] | long_seed[t + 1] << 8);
		t = t + 2;

		pos = (upper / (i + 511)) * (i + 511);
		if (keep < pos)
		{
			pos = keep % (i + 511);
			f[i + 511] = f[pos];
			f[pos] = 1;
			i++;
		}
	}

	for (i = 0; i < d;)
	{
		if (t >= seed_len) {
			t = 0;
			shake128_squeezeblocks(long_seed, 1, &state);
			seed_len = SHAKE128_RATE; // 168
		}

		keep = (long_seed[t] | long_seed[t + 1] << 8);
		t = t + 2;

		pos = (upper / (768 + i)) * (768 + i);
		if (keep < pos)
		{
			pos = keep % (768 + i);
			f[768 + i] = f[pos];
			f[pos] = -1;
			i++;
		}
	}

}



/* every coef of a is uniform from {-alpha/2,,...,alpha/2-1} */
/*
	input: polynomial pointer
		   poly length
		   random byte array pointer
		   random byte array length
 */
int sample_rej(
	int32_t *a, 
	unsigned int len, 
	const unsigned char *buf, 
	unsigned int buflen)  // 2184+168
{

	int32_t bit[168 * 8];
	for (int j = 0; j < 168;j++){
		unsigned char temp = buf[2184 + j];
		for (int k = 0; k < 8;k++){
			bit[8 * j + k] = temp & 1;
			temp = (temp >> 1);
		}
	}
	int fill = 0;  // record current number of evaluated coefficient
	int consume = 0; // number of consumed random bytes
	int bit_consume = 0;
	int32_t t = 0;
	while(consume<buflen-168 && fill<len && bit_consume<168*8) 
	{
		t = buf[consume++];
		t |= buf[consume++] << 8;
		t |= bit[bit_consume++] << 16;
		if(t < ALPHA){
			a[fill++]=t-(ALPHA>>1);
		}
	}
	return fill;
}

void sample_r(int32_t *a) {
	int ctr;
	unsigned char short_seed[32];
	unsigned char buf[14*168],buf1[2*168];

	keccak_state state;
	rand_byts(32, short_seed);
	uint16_t nonce = (short_seed[0] << 8) | short_seed[1];
	
	shake128_stream_init(&state, short_seed, nonce);
	shake128_squeezeblocks(buf, 14, &state);

	ctr = sample_rej(a, N, buf, 14*168);
	while (ctr < N) {
		shake128_squeezeblocks(buf1, 2, &state);
		ctr += sample_rej(a + ctr, N - ctr, buf1, 2*168);
	}
}
void neg_wrapped_convulution(const int32_t *a, const int32_t *b, int32_t *c){
    int64_t sum1;
    int64_t sum2;
    for (int i = 0; i < N; i++)
    {
        sum1 = 0;
        sum2 = 0;
        for (int j = 0; j <= i; j++)
        {
            sum1 = (sum1 + (int64_t)a[j] * b[i - j]%Q) % Q;
        }
        for (int j = i + 1; j < N; j++){
            sum2 = (sum2 + ( (int64_t)(Q-a[j]) * b[N + i - j]% Q) ) % Q;
        }
        c[i] = (int32_t)((sum1 + sum2) % Q);
    }
}
void poly_compare(int *a, int *b){
	for (int i = 0; i < 10;i++)
		if (a[i] != b[i])
		{
			printf("%d: %d\t%d  diff=%d\n", i, a[i], b[i],a[i]-b[i]);
		}
}

void eval_poly(int32_t *f, int32_t *g)
{
	int i;
	for (i = 0; i < N; i++) 
		f[i] = g[i];
}

/* Compute the inverse of a positive number mod Q */
int32_t inv_num(int32_t n)
{
	int32_t  u, v, v1, v2, q, r, v0;

	v1 = 0;
	v2 = 1;
	u = Q; v = n;

	while (v != 0) {
		q = u / v;
		r = u % v;
		u = v;
		v = r;
		v0 = v2;
		v2 = v1 - q * v2;
		v1 = v0;
	}
	return (v1+Q);
}

/**
 * make coef from {0,...,q-1} to { -(q-1)/2,...,-(q-1)/2 }
 */
void balance(int32_t *f)
{
	int i;
	for (i = 0; i < N; i++)
	{
		f[i] = f[i] - (f[i] / QP)*Q;
	}
}

// void balance_r(int32_t *r){
// 	for (int i = 0; i < N;i++)
// 	{
// 		if(r[i]>=ALPHA/2){
// 			r[i] -= ALPHA;
// 		}	
// 	}
// }

/**
 * judge whether inf norm of g <= a
 * if not exceed return 0
 */
int judge(int32_t *g, int a) {
	int i, ctr = 0;

	for (i = 0; i < N; i++) {
		ctr = ctr ||(g[i]/ (a + 1));
	}
	if (ctr == 1) return 1;
	return 0;
}




void eq(int *a, int *b, int q){
  int flag = 0;
  for (int i = 0; i < N;i++){
    if((a[i]-b[i])%q){
      flag = 1;
      printf("%d: %d\t%d\n", i, a[i], b[i]);
    }
  }
  if(flag)
    return;
  printf("same\n");
}


void display(int32_t *arr){
	printf("{");
	for (int i = 0; i < N; i++){
		printf("%d, ", arr[i]);
	}
	printf("}\n\n");
}




/*
 * Input f, if there is a f' such that ff' = 1 in Rq
 * NTT(ff')=NTT(1)=[1,1,...,1]
 * If f is invertible, then NTT(f)[i] must not be zero.
 * Compute NTT(f')= NTT(f)^(-1) mod q and store it in fq
 * return 1 if not invertible
*/
int inv_poly(int32_t *f, int32_t *fq)
{
	int ctr;
	int32_t g[N], i,j;


	for (i = 0; i < N; i++) {
		g[i] = f[i];
	}

	poly_ntt_o(g); // g[i] in Zq={0,1,...,q-1}
	ctr = 0;
	for (i = 0; i < N; i++) {
		ctr = ctr | ((g[i] - 1) >> 21);
	}
	if (ctr != 0) return 1;
	for (i = 0; i < N; i++)
		fq[i] = inv_num(g[i]);
	return 0;
}


void poly_add(int32_t *a, int32_t *b, int32_t *c) {
	for (int i = 0; i < 1024; i++) {
		c[i] = a[i] + b[i];
	}
}

/* f = x^k  times g */
void right_shift(int32_t *f, const int32_t *g, int k) {
	int i;
	for (i = 0; i < 1024 - k; i++) {
		f[k + i] = g[i];
	}
	for (i = 0; i < k; i++) {
		f[k - 1 - i] = -g[1024 - 1 - i];
	}
}


/**
 * description: multiplication of a fixed weighted binary and a trinary polynomial
 * arr: array of locations of 1 as input
 * f: a trinary poly as input
 * res: the output product
 * sz: length of array as input
 */
 void sparse_times(uint16_t *arr, int32_t *f, int32_t *res, int32_t sz) {
	int i;
	memset(res, 0, 1024 * sizeof(int32_t));
	int y[1024];
	for (i = 0; i < sz; i++)
	{
		right_shift(y, f, arr[i]);
		poly_add(y, res, res);
	}
}