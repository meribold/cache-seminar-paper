/* vim: set tw=72 sts=-1 sw=3 et cms=//%s: */

#include "stdint.h"  // int64_t
#include "stdio.h"   // printf
#include "stdlib.h"  // calloc
#include "time.h"    // clock

// Adapted from <igoro.com/archive/gallery-of-processor-cache-effects/>.
//
// Apparently malloc(3) can return non-NULL WITHOUT HAVING ACTUALLY
// ALLOCATED ANY MEMORY on Linux!  Even when requesting MORE MEMORY THAN
// THE SUM OF THE SYSTEM'S PHYSICAL MEMORY AND SWAP SPACE!  ...use
// `calloc`.
//
// Multiplying the array entries (`array[i] *= 3`) gives better results
// than incrementing (`++array[i]`) them.  The processor times for runs
// that need every cache line are more even.  Also, multiplying with
// uneven numbers gives better results than with even ones.  For a step
// size of 1 (and only for a step size of 1), the multiplication is
// actually faster than something like `array[i] &= 1`.  This depends on
// having optimization enabled.
//
// Generally, results are more predictable when the system is idle.
// Rebooting also helped before when I got strange results (almost the
// same times for step sizes of 32 and 64).

#define SIZE 67108864  // 64 * 1024 * 1024.  The array will be 512 MiB.

int main() {
   int64_t* array = (int64_t*)calloc(SIZE, sizeof(int64_t));
   clock_t t0 = clock();
   for (size_t i = 0; i < SIZE; i += STEP) {
      array[i] &= 1;  // Do something.  Anything.
   }
   clock_t t1 = clock();
   printf("%d %f\n", STEP, 1000. * (t1 - t0) / CLOCKS_PER_SEC);
}
