#ifndef PARAMS_H
#define PARAMS_H

#define N 1024
#define Q 1038337            //  Log2[1038337]= 19.9858
#define P 8
#define D 256               //T(d+1,d): d+1 ones, d negative ones
#define RLEN 32
#define L 44
#define GAMMA 20
#define HASHBIT 32           
#define ALPHA  129792  // Log2[129792]=16.9858   (q-1)/p
#define MLEN 256       // 
#define ULEN 2176

#define  W_MOD_P_BYTE_LEN 384      //(3*N/8)

#define N2 512
#define N4 256
#define N8 128

#define TRI_POLY_BYTE_LEN 256

#define HASH_INPUT_LEN (32+256+384)

#define QP ((Q+1)/2)    
#define LOGQ 20                // each coeff occupies 20 bits 
#define N_BYTES 2560        //(LOGQ*N)/8 = 20 * 1024 / 8 = 2560 bytes
#define L_BYTES ((10*L)>>3)           //(10*L)/8 = 10 * 44 /8 = 55 bytes

#endif