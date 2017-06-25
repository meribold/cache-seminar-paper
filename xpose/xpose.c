#include <stdint.h>  // size_t
#include <stdio.h>   // printf
#include <stdlib.h>  // calloc, `int rand()`, srand
#include <time.h>    // clock

// #define m 5000
// #define n 5000

#ifdef SIMPLE
void transpose(int D[m][n], int E[n][m]) {
   for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j) E[j][i] = D[i][j];
}
#else
void transpose(int I[2], int J[2], int D[m][n], int E[n][m]) {
   int num_rows = 1 + I[1] - I[0];
   int num_cols = 1 + J[1] - J[0];
   if (num_cols == 1 && num_rows == 1) {
      E[J[0]][I[0]] = D[I[0]][J[0]];
   } else if (num_cols <= num_rows) {
      // Horizontally slice D into two submatrices and recurse.
      transpose((int[2]){I[0], I[0] + num_rows / 2 - 1}, J, D, E);
      transpose((int[2]){I[0] + num_rows / 2, I[1]}, J, D, E);
   } else {
      // Vertically slice D into two submatrices and recurse.
      transpose(I, (int[2]){J[0], J[0] + num_cols / 2 - 1}, D, E);
      transpose(I, (int[2]){J[0] + num_cols / 2, J[1]}, D, E);
   }
}
#endif

void print_matrix(int rows, int cols, int D[rows][cols]) {
   if (rows > 42 || cols > 42) return;
   for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
         printf("%d ", D[i][j]);
      }
      printf("\n");
   }
}

int main() {
   int(*D)[n] = (int(*)[n])calloc(n * m, sizeof(int));
   srand(time(0));
   for (size_t i = 0; i < n * m; ++i) {
      ((int*)D)[i] = rand();
   }
   int(*E)[m] = (int(*)[m])calloc(n * m, sizeof(int));
   // int D[m][n] = {{1, 2}, {3, 4}, {5, 6}};
   // int E[n][m];
   // print_matrix(m, n, D);
   // printf("\n");
   clock_t t0 = clock();
#ifdef SIMPLE
   transpose(D, E);
#else
   transpose((int[2]){0, m - 1}, (int[2]){0, n - 1}, D, E);
#endif
   clock_t t1 = clock();
   // print_matrix(n, m, E);
   printf("%ld %f\n", (m * n) * sizeof(int) / 1024,
          1000. * (t1 - t0) / CLOCKS_PER_SEC);
}

// vim: tw=72 sts=-1 sw=3 et cms=//%s
