#include "sig.h"


// w = j * ALPHA + r = j * ALPHA + ALPHA / 2 +R(w)

 const int32_t A[8] = { 
	 0 + ALPHA / 2,
	 ALPHA + ALPHA / 2,
	 2 * ALPHA + ALPHA / 2,
	 3 * ALPHA + ALPHA / 2,
	 4 * ALPHA + ALPHA / 2,
	 5 * ALPHA + ALPHA / 2,
	 6 * ALPHA + ALPHA / 2,
	 7 * ALPHA + ALPHA / 2 };

unsigned long long sig_get_pk_byts()
{
	return (N_BYTES);
}

unsigned long long sig_get_sk_byts()
{
	return (N_BYTES + (N2));
}

unsigned long long sig_get_sn_byts()
{
	return (ULEN + RLEN + L_BYTES);
}


/* B_l: 1024-dim binary string, wighted 44 */
/* input 32 bytes = 256 bits short seed */
/* B_l: 1024-dim binary string, wighted 44 */
/* input 32 bytes = 256 bits short seed */
void hash_to_fixed_weight(int l, unsigned char* short_seed, uint16_t *arr) {
	int i,j;
	int16_t f[1024];
	memset(f, 0, sizeof(int16_t) * 1024);
	unsigned char long_seed[168];
	keccak_state state;
	uint16_t pos;
	uint64_t five_bytes_to_four_pos[33];
	uint16_t nonce = (short_seed[0] << 8) | short_seed[1];
	shake128_stream_init(&state, short_seed, nonce);
	int rec = 0;

	while (rec < l) {
		shake128_squeezeblocks(long_seed, 1, &state);
		for (i = 0; i < 11; i++) 
		{
			five_bytes_to_four_pos[i] = (uint64_t)long_seed[5 * i] | ((uint64_t)long_seed[5 * i + 1] << (8)) | ((uint64_t)long_seed[5 * i + 2] << (16)) | ((uint64_t)long_seed[5 * i + 3] << (24)) | ((uint64_t)long_seed[5 * i + 4] << (32));
			for (j = 0; j < 4; j++)
			{
				pos = five_bytes_to_four_pos[i] & 0x3FFU;
				five_bytes_to_four_pos[i] >>= 10;
				if (f[pos] == 0) 
				{
					f[pos] = 1;
					arr[rec++] = pos;
				}
			}
		}
		if (rec == 44)
			return;
		for (i = 11; i < 33; i++)
		{
			five_bytes_to_four_pos[i] = (uint64_t)long_seed[5 * i] | ((uint64_t)long_seed[5 * i + 1] << (8)) | ((uint64_t)long_seed[5 * i + 2] << (16)) | ((uint64_t)long_seed[5 * i + 3] << (24)) | ((uint64_t)long_seed[5 * i + 4] << (32));
			for (j = 0; j < 4; j++)
			{
				pos = five_bytes_to_four_pos[i] & 0x3FFU;
				five_bytes_to_four_pos[i] >>= 10;
				if (f[pos] == 0)
				{
					f[pos] = 1;
					arr[rec++] = pos;
					if (rec == 44)
						return;
				}
			}
		}

		pos = long_seed[165] | ((long_seed[166] & 3) << 8); // 8+2
		if (f[pos] == 0)
		{
			f[pos] = 1;
			arr[rec++] = pos;
			if (rec == 44)
				return;
		}
		pos = (long_seed[166] >> 2) | ((long_seed[167] & 15) << 6); //6+4
		if (f[pos] == 0)
		{
			f[pos] = 1;
			arr[rec++] = pos;
			if (rec == 44)
				return;
		}

	}

}


//输入 chtohash 生成 固定重量的hash 存储其1的位置
/**********************
 * input chtohash of length inputlen
 * hash chtohash 
 * output the position of 1 in uint16_t array
 **********************/
void hash_pack(
	uint16_t *ad, 
	unsigned char *chtohash, 
	int inputlen) 
{
	unsigned char a_bytes[HASHBIT];
	shake256(a_bytes, HASHBIT, chtohash, inputlen);
	hash_to_fixed_weight(44, a_bytes, ad);
}


/*  output:
 * pk byte array of NTT(h)
 * sk byte array of g || f || NTT(h)
 * coef of a trinary uses 2 bits to represent
 * 2*1024/8 = 256
*/
int sig_keygen(
	unsigned char *pk, 
	unsigned char *sk)
{
	int i;
	int32_t f[N], g[N], fq[N], h[N];
	//printf("in KG\n");
	//generate an invertible f
	trinary_poly_gen(f);
	//printf("f is \n");
	//print_poly(f);

	while (inv_poly(f, fq) == 1)
		trinary_poly_gen(f);

		trinary_poly_gen(g);
	//print_poly(g);
	trinary2byteArray(sk, g);   // sk byte array first part g of length 256

	g[0] = g[0] + ALPHA;
	poly_ntt_o(g);
	poly_pointwise_o(h, g, fq);  // h: polynomial form

	poly_tobytes(pk, h);		   // pk: byte array of h
	trinary2byteArray(sk + N4, f); // sk byte array second part f of byte length 256
	for (i = 0; i < N_BYTES; i++)
		*(sk + N2 + i) = *(pk + i); // sk byte array third part NTT(h) of byte length 2432

	return 0;
}

/**
 * input sk byte array, message array
 * output signature sigma = (k, u , a)
 */
int sig_sign(
	unsigned char *sk, 
	const unsigned char *m, 
	unsigned char *sigma)
{
	int32_t f[N], g[N], h[N],  w[N],  w_mod_p[N];
	int32_t af[N], ag[N];
	int32_t af_plus_r[N], pag_plus_w[N], ag_plus_Rw[N];
	int32_t r[N] = {0};
	int32_t rcopy[N] = {0};
	int32_t qw[N];
	unsigned char qw_byte_array[W_MOD_P_BYTE_LEN]={0};

	//\innorm{ p{ag}+w }>\frac{q}{2}-p\cdot l

	unsigned char seed[32], k[RLEN];

	// Hash(k,M,w mod p)
	unsigned char chtohash[RLEN + MLEN + W_MOD_P_BYTE_LEN];

	uint16_t a_pos[L];
	int i;


	//recover the polynomial sk and pk
	byteArray2trinary(g, sk);
	byteArray2trinary(f, sk + TRI_POLY_BYTE_LEN);
	poly_frombytes(h, sk + TRI_POLY_BYTE_LEN * 2);

	//generate k	
	rand_byts(32, seed);
	shake256(k, RLEN, seed, 32);
	
	//compute the input of the hash (k||m|| w mod p) 
	
	for (i = 0; i < RLEN; i++)
		chtohash[i] = k[i];
	for (i = 0; i < MLEN; i++)
		chtohash[i + RLEN] = m[i];



pos_1:
	memset(r, 0, N * sizeof(int32_t));
	//generate r
	sample_r(r);  // {-alpha/2,...,alpha/2-1}

	eval_poly(rcopy, r);
	poly_ntt_o(rcopy);  // NTT(r)
	poly_pointwise_o(w, h, rcopy);   // NTT(h)NTT(r)
	poly_invntt_o(w); // w = hr mod q, w[i] in {0,..., q-1}

	

	// compute Q(w)
	//printf("here\n");
	for (i = 0; i < N; i++)
	{
		qw[i] = (w[i] - 1) / (ALPHA);
		//printf("%d, ", qw[i]);
	}
	//printf("\n");

	//generate a=Hash(k,M,w mod p)
	coeff3bit_to_byte_array(qw_byte_array, qw);

	

	for (i = 0; i < W_MOD_P_BYTE_LEN; i++)
		chtohash[i + RLEN + MLEN] = qw_byte_array[i];

	hash_pack(a_pos, chtohash, RLEN + W_MOD_P_BYTE_LEN + MLEN);

	sparse_times(a_pos, f, af, L); // af = a*f, coef in [-l,l]
	for (i = 0; i < N; i++)
	{
		af_plus_r[i] = af[i] + r[i]; // af+r, coef in [-alpha/2-l,alpha/2+l)
	}


	if (judge(af_plus_r, ALPHA / 2 - L - 1)) goto pos_1;
	// keep af+r satisfyting that coef in (-alpha/2+l, alpha/2-l)
	//printf("check af+r \n");

	sparse_times(a_pos, g, ag, L);  //ag

	for (i = 0; i < N; i++){
		ag_plus_Rw[i] = ag[i] + (w[i] - A[qw[i]]);
	}

	if (judge(ag_plus_Rw, ALPHA / 2 - L - 1)) goto pos_1;
	//printf("check ag+R(w)\n");

	// signature  ( k || a || u=af+r )
	for (i = 0; i < RLEN; i++)
		sigma[i] = k[i];
	poly_tobytes_ten(sigma + RLEN, a_pos, L);
	poly_tobytes_u(sigma + L_BYTES + RLEN, af_plus_r);

	// int w_plus_ag[N] = {0};
	// int w_plus_ag2[N] = {0};
	// int hu[N] = {0};
	// for (i = 0; i < N;i++){
	// 	w_plus_ag2[i] = w[i];
	// }
	// for (int i = 0; i < 10 ;i++)
	// 	printf("%d\t", ag[i]);
	// printf("\n");
	// poly_ntt_o(af_plus_r);
	// poly_pointwise_o(w_plus_ag, h, af_plus_r);
	// poly_invntt_o(w_plus_ag);
	// poly_invntt_o(h);
	// poly_invntt_o(af_plus_r);
	// neg_wrapped_convulution(h, af_plus_r, hu);
	// for (i = 0; i < L; i++)
	// {
	// 	//w[a_pos[i]] =   (w[a_pos[i]]-1+Q)%Q;
	// 	hu[a_pos[i]] = (hu[a_pos[i]] - ALPHA + Q) % Q;
	// }
	// poly_compare(w_plus_ag2, hu);
	//printf("exit signing\n");
	return 0;

	
}

int sig_verf(
	unsigned char *pk,
	unsigned char *sigma,
	unsigned char *m)
{
	int32_t  h[N], u[N], w[N], w_mod_p[N];
	uint16_t a_pos[L], adp[L];
	//uint8_t  w_mod_p_byte_array[ W_MOD_P_BYTE_LEN ]={0};
	unsigned char qw_byte_array[W_MOD_P_BYTE_LEN]={0};
	unsigned char   hash[ RLEN +  MLEN + W_MOD_P_BYTE_LEN ];
	int  i;

	//load the pk and the sig
	poly_frombytes(h, pk);   // recover NTT(h)
	
	poly_frombytes_ten(a_pos, sigma + RLEN, L);
	poly_frombytes_u(u, sigma + (L_BYTES + RLEN));

	//check the norm of u
	if (judge(u, ALPHA / 2 - L - 1))
	{
		printf("u的长度\n");
		return 0;
	}


	//generate the input of  the hash
	poly_ntt_o(u);
	poly_pointwise_o(w, h, u);
	poly_invntt_o(w);
	
	for (i = 0; i < L; i++)
	{
		//w[a_pos[i]] =   (w[a_pos[i]]-1+Q)%Q;
		w[a_pos[i]] = (w[a_pos[i]] - ALPHA + Q) % Q;
	}


	int32_t rw[N],qw[N];
	for (i = 0; i < N; i++){
		qw[i] = (w[i] - 1) / ALPHA ;
		rw[i] = w[i] - A[qw[i]];
	}
	//print_poly(qw);

	//check the norm of hu-ALPHA*a mod q mod p
	if (judge(rw, ALPHA / 2 - L - 1))
	{
		printf("余数条件\n");
		return 0;
		}

	

			coeff3bit_to_byte_array(qw_byte_array, qw);

		for (i = 0; i < RLEN; i++)
			hash[i] = sigma[i];
		for (i = 0; i < MLEN; i++)
			hash[i + RLEN] = m[i];
		for (i = 0; i < W_MOD_P_BYTE_LEN; i++)
			hash[i + RLEN + MLEN] = qw_byte_array[i];


		//generate the hash and check
		hash_pack(adp, hash, RLEN + MLEN + W_MOD_P_BYTE_LEN);
	
	for (i = 0; i < L; i++)
	{
		if (adp[i] != a_pos[i]) 
		{	printf("hash equality NOT hold\n");	
			return 0;
		}
	}
	return 1;
}
