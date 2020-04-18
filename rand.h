#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H
unsigned long long rand_get_sd_byts();
int rand_init(unsigned char*s, unsigned long long s_bytes);
int rand_byts(unsigned long long buflen, unsigned char *buf);




#endif
