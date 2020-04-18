#ifndef BYTE_H
#define BYTE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "poly.h"
#include "params.h"

void poly_frombytes(int32_t * p, const unsigned char *octet);
void poly_tobytes(unsigned char *octet, const int32_t *poly);

void coeff3bit_to_byte_array(unsigned char *octet, int *poly);

void poly_tobytes_ten(unsigned char *a, const uint16_t *f, const int32_t polylen);
void poly_frombytes_ten(uint16_t * f, const unsigned char *a, const int32_t polylen);

void trinary2byteArray(unsigned char *b, const int32_t *f);

void byteArray2trinary(int32_t *f, unsigned char *b);
void byteArray2binary(int32_t *m, const unsigned char *b, const int32_t n);

void poly_frombytes_qw(uint16_t * p, const unsigned char *octet);
void poly_tobytes_qw(unsigned char *octet, uint16_t *p);

void poly_frombytes_u(int32_t * p, const unsigned char *octet);
void poly_tobytes_u(unsigned char *octet, const int32_t *p);

#endif