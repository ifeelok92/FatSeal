#ifndef NTT_H
#define NTT_H

#include "params.h"
#include <stdint.h>
#include "poly.h"

int32_t montgomery_reduce(int64_t a);
int32_t barrett_reduce(int32_t a);

void poly_ntt_o(int32_t *p);
void poly_invntt_o(int32_t * a);
void poly_pointwise_o(int32_t * c, int32_t * a, int32_t * b);



#endif