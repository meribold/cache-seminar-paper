// vim: tw=72 sts=-1 sw=3 et

#include <cstdlib>  // std::rand, std::srand
#include <ctime>    // std::clock, std::time, std::clock_t
#include <iostream>

#ifdef VECTOR
#include <vector>
#else
#include <list>  // Or <forward_list>?
#endif

#ifdef VECTOR
using Container = std::vector<int>;
#else
using Container = std::list<int>;
#endif

// When I try to lay the list out sequentially in memory, the difference
// changes to a factor of about 11.
//
//    for (int n = 0; n < N; ++n) {
//       containers[n].resize(5000, std::rand());
//       for (int i = 0; i < 5000; ++i) {
//          containers[n].push_back(std::rand());
//       }
//    }

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
      for (int num : containers[m]) {
         sum += num;
      }
   }
   std::clock_t t1 = std::clock();

   // Also print the sum so the loop doesn't get optimized out.
   std::cout << sum << '\n' << (t1 - t0) << '\n';
}
