// vim: tw=72 sts=-1 sw=3 et

#include <cstdlib>  // std::rand, std::srand
#include <ctime>    // std::clock, std::time, std::clock_t
#include <iostream>
#include <list>

#include <xmmintrin.h>

// No prefetching can save this program.

using Container = std::list<int>;

constexpr int N = 5000;

int main() {
   Container containers[N];
   std::srand(std::time(nullptr));
   // Append an average of 5000 random values to each container.
   for (int i = 0; i < N * 5000; ++i) {
      containers[std::rand() % N].push_back(std::rand());
   }

   int sum = 0;
   std::clock_t t0 = std::clock();
   for (int m = 0; m < N; ++m) {
      auto it = containers[m].begin();
      auto next = std::next(it, 34);
      auto end = containers[m].end();
      while (it != end) {
         ++next;
         _mm_prefetch(next._M_node, _MM_HINT_T0);
         sum += *it;
         ++it;
      }
   }
   std::clock_t t1 = std::clock();

   // Also print the sum so the loop doesn't get optimized out.
   std::cout << sum << '\n' << (t1 - t0) << '\n';
}
