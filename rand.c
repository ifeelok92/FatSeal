#include "rand.h"
#include "rng.h"
//#include<intrin.h>


unsigned long long rand_get_sd_byts()
{
  
  return  48;

}



int rand_init(unsigned char*s, unsigned long long s_bytes)
{
     if(s==NULL) return -1;
	randombytes_init(s,NULL,256);

	return 0;
}

int rand_byts(unsigned long long buflen, unsigned char *buf)
{   

	if(buf==NULL) return -1;
	randombytes(buf, buflen);
	return 0;
}