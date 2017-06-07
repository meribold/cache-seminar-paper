// vim: tw=72 sts=-1 sw=3 et

#include <cassert>
#include <iostream>

#ifdef VECTOR
#include <vector>
#else
#include <list>
#endif

using namespace std;

#ifdef VECTOR
using Container = vector<int>;
#else
using Container = list<int>;
#endif

constexpr int N = 5000;

int main() {
   Container containers[N];
   srand(time(0));
   // Append an average of 5000 random values to each container.
   for (int i = 0; i < N * 5000; ++i) {
      containers[rand() % N].push_back(rand());
   }

   int sum = 0;
   clock_t t0 = clock();
   for (int m = 0; m < N; ++m) {
      for (int num : containers[m]) {
         sum += num;
      }
   }
   clock_t t1 = clock();

   // Print the sum so the loop doesn't get optimized out.
   cout << sum << '\n';
   cout << (t1 - t0) << '\n';
}
