/* vim: set tw=72 sts=-1 sw=3 et: */

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#define N 5000

int64_t array[SIZE];

int main() {
   srand(time(0));
   for (size_t i = 0; i < SIZE; ++i) {
      array[i] = rand();
   }

   clock_t t0 = clock();
   int64_t sum = 0;
   for (size_t n = 0; n < N; ++n) {
      for (size_t i = 0; i < SIZE; ++i) {
         sum += array[i];
      }
   }

   clock_t t1 = clock();
   printf("%d %f\n", SIZE * 8 / 1024,
          1000. * (t1 - t0) / CLOCKS_PER_SEC);
   // Print the sum so the loop doesn't get optimized out!
   printf("%d\n", sum);
}
