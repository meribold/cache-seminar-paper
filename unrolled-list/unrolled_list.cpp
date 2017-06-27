// vim: tw=72 sts=-1 sw=3 et

#include <cassert>
#include <cstdlib>  // std::rand, std::srand
#include <ctime>    // std::clock, std::time, std::clock_t
#include <iostream>

#include <xmmintrin.h>

#ifdef UNROLL
#include <array>
#include "list"  // Include the copy in this directory.
#else
// #include <list>
#include <vector>
#endif

constexpr int NODE_SIZE = 12;
// constexpr int NODE_SIZE = 28;
// constexpr int NODE_SIZE = 44;
// constexpr int NODE_SIZE = 60;

#ifdef UNROLL
// Extremely ad-hoc.
class unrolled_list : public std::list<std::array<int, NODE_SIZE>> {
  public:
   void push_back(const int& x) {
      // _Node is a typedef for the actual node type GCC uses.
      // assert(sizeof(_Node) == 64);
      // std::cout << alignof(_Node) << '\n';
      if (!this->empty()) {
         // std::cout << long(this->end()._M_node) % 64 << '\n';
         auto& last = this->back();
         int& num_elements = last[0];
         assert(num_elements >= 1);
         // std::cout << last[0] << std::endl;
         if (num_elements < NODE_SIZE - 1) {
            // if (num_elements < 2) {
            ++num_elements;
            last[num_elements] = x;
            return;
         }
      }
      value_type new_node;
      new_node[0] = 1;  // One element.
      new_node[1] = x;
      std::list<value_type>::push_back(new_node);
   }
};
using Container = unrolled_list;
#else
// using Container = std::list<int>;
using Container = std::vector<int>;
#endif

constexpr int N = 5000;

int main() {
   Container containers[N];
   std::srand(std::time(nullptr));
   // Append an average of 5000 random values to each container.
   for (int i = 0; i < N * 5000; ++i) {
      containers[std::rand() % N].push_back(std::rand());
   }
   // for (int i = 0; i < N; ++i)
   //    for (int j = 0; j < 5000; ++j)
   //       containers[i].push_back(std::rand());

   int sum = 0;
   std::clock_t t0 = std::clock();
   for (int m = 0; m < N; ++m) {
#ifdef UNROLL
      auto it = containers[m].begin();
      auto next = std::next(it, 4);
      auto end = containers[m].end();
      it = containers[m].begin();
      for (; it != end;) {
         ++next;
         _mm_prefetch(next._M_node, _MM_HINT_T0);
         // _mm_prefetch((char*)(next._M_node) + 64, _MM_HINT_T0);
         auto& num = *it;
         // _mm_prefetch((void*)(it._M_node) + 64, _MM_HINT_T0);
         // _mm_prefetch(&num[12], _MM_HINT_T0);
         ++it;
         // _mm_prefetch(&(*it)[12], _MM_HINT_T0);
         // _mm_prefetch((char*)(it._M_node) + 128, _MM_HINT_T0);
         // _mm_prefetch((char*)(it._M_node) + 256, _MM_HINT_T0);
         int i = 0;
         for (; i < num[0]; ++i) {
            sum += num[i];
         }
         // for (; i < num[0] / 2; ++i) {
         //    sum += num[i];
         //    // sum /= 42;
         // }
         // _mm_prefetch((char*)(it._M_node) + 64, _MM_HINT_T0);
         // for (; i < num[0]; ++i) {
         //    sum += num[i];
         // }
      }
#else
      for (int num : containers[m]) {
         sum += num;
         // sum /= 42;
      }
#endif
   }
   std::clock_t t1 = std::clock();

   // Also print the sum so the loop doesn't get optimized out.
   std::cout << sum << '\n' << (t1 - t0) << '\n';
}
