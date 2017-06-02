// vim: tw=72 sts=-1 sw=3 et cms=//%s

#include <stdint.h>  // int64_t, size_t
#include <stdio.h>   // printf
#include <stdlib.h>  // rand

#define N 100000000  // 100 million

struct elem {
   struct elem *next;
} array[SIZE];

int main() {
#ifndef STEP
   for (size_t i = 0; i < SIZE - 1; ++i) array[i].next = &array[i + 1];
   array[SIZE - 1].next = array;
#else
   for (size_t i = 0; i < SIZE - STEP; i += STEP)
      array[i].next = array + i + STEP;
   array[SIZE - STEP].next = array;
#endif
#ifndef BASELINE
   int64_t dummy = 0;
   struct elem *i = array;
   for (size_t n = 0; n < N; ++n) {
      // dummy /= 73;
      dummy += (int64_t)i;
      // dummy /= 8234;
      i = i->next;
   }
   printf("%ld\n", dummy);
#endif
}
