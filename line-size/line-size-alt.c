/* vim: set tw=72 sts=-1 sw=3 et cms=//%s: */

#include "stdint.h"  // int64_t
#include "stdio.h"   // printf
#include "stdlib.h"  // calloc
#include "time.h"    // clock

#define SIZE 67108864  // 64 * 1024 * 1024.  The array will be 512 MiB.

int64_t array[SIZE];  // Using `int8_t` or `int32_t`: bad.

int main() {
   // int64_t* array = (int64_t*)malloc(sizeof(int64_t) * SIZE);
   // This just forces the memory to really be allocated before we
   // measure.
   for (size_t i = 0; i < SIZE; ++i) array[i] = 1;
   struct timespec t0, t1;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
   for (size_t i = 0; i < SIZE; i += STEP) {
      array[i] *= 3;
   }
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
   printf("%d %f\n", STEP,
          1.e3 * (t1.tv_sec - t0.tv_sec) +
              1.e-6 * (t1.tv_nsec - t0.tv_nsec));
}
