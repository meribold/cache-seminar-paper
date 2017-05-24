// vim: set tw=72 sts=-1 sw=3 et cms=//%s

#include "stdint.h"  // int64_t
#include "stdio.h"   // printf
#include "stdlib.h"  // calloc

#define N 1000000000

int main() {
   int64_t* array = (int64_t*)calloc(SIZE, sizeof(int64_t));
   int sum;
   for (size_t n = 0; n < N; ++n) {
      sum += array[(8 * n) % SIZE];
   }
   printf("%d\n", sum);  // Better do this...
}
