// vim: tw=72 sts=-1 sw=3 et

#include <thread>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cstdlib>  // std::rand
#include <ctime>    // std::clock, std::time, std::clock_t
#include <chrono>   // std::chrono::high_resolution_clock

constexpr int DIM = 20000;

int main() {
   auto matrix_ptr = std::make_unique<std::array<int, DIM * DIM>>();
   auto& matrix = *matrix_ptr;
   std::srand(std::time(nullptr));
   for (std::size_t i = 0; i < DIM * DIM; ++i) {
      matrix[i] = std::rand();
   }

   auto t0 = std::chrono::high_resolution_clock::now();
   int result[THREADS];
   std::vector<std::thread> threads;
   // Each of THREADS parallel workers processes 1/THREADS-th of the
   // data; the p-th worker records it's partial count in result[p]
   for (std::size_t p = 0; p < THREADS; ++p)
      threads.push_back(std::thread([&, p] {
         result[p] = 0;
         int count = 0;  // XXX
         int chunkSize = DIM / THREADS + 1;
         int myStart = p * chunkSize;
         int myEnd = std::min(myStart + chunkSize, DIM);
         for (int i = myStart; i < myEnd; ++i)
            for (int j = 0; j < DIM; ++j)
               // if (matrix[i * DIM + j] % 2 != 0) ++result[p];
               if (matrix[i * DIM + j] % 2 != 0) ++count;
         result[p] = count;  // XXX
      }));
   // Wait for the parallel work to complete...
   for (auto& thread : threads) thread.join();
   // Finally, do the sequential "reduction" step to combine the results
   int odds = 0;
   for (int p = 0; p < THREADS; ++p) odds += result[p];
   auto t1 = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double, std::milli> duration = t1 - t0;
   std::cout << odds << '\n' << duration.count() << '\n';
}
