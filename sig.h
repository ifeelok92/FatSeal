#ifndef SIG_H
#define SIG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "params.h"
#include "fips202.h"
#include "byte.h"
#include "poly.h"
#include "rand.h"
int sig_keygen(
	unsigned char *pk,
	unsigned char *sk);

int sig_sign(
	unsigned char *sk,
	const unsigned char *m,
	unsigned char *sigma);

int sig_verf(
	unsigned char *pk,
	unsigned char *sigma,
	unsigned char *m);
#endif