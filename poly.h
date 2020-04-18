#ifndef POLY_H
#define POLY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fips202.h"
#include "params.h"
#include "ntt.h"
#include "rng.h"
#include "rand.h"

void print_poly(int32_t *f);
void balance_r(int32_t *r);

void trinary_poly_gen(int32_t  *f);
void sample_r(int32_t *a);
void eval_poly(int32_t *f, int32_t *g);
int32_t inv_num(int32_t n);
void balance(int32_t *f);
int judge(int32_t *f, int a);
int inv_poly(int32_t *f, int32_t *fq);
void eq(int *a, int *b, int q);
void display(int32_t *arr);
void poly_add(int32_t *a, int32_t *b, int32_t *c);
/* f = x^k * g */
void right_shift(int32_t *f, const int32_t *g, int k);
void poly_compare(int *a, int *b);
void neg_wrapped_convulution(const int32_t *a, const int32_t *b, int32_t *c);
/* arr: array of locations of 1 of polynomial a, f is a trinary poly */
void sparse_times(uint16_t *arr, int32_t *f, int32_t *res, int32_t sz);
#endif