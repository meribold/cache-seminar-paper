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
#elif defined COMPLICATED
int isolate_rightmost_bit(int i) { return i & (-i); }

int is_power_of_two(int i) {
   // See <https://stackoverflow.com/q/108318>.
   return (i == 1) | ((i & (i - 1)) == 0);
}

// From <http://asgerhoedt.dk/?p=276>.
unsigned int compact(int x) {
    x &= 0x55555555;                 // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    x = (x ^ (x >> 1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
    x = (x ^ (x >> 2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
    x = (x ^ (x >> 4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
    x = (x ^ (x >> 8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
    return x;
}

// From <http://asgerhoedt.dk/?p=276>.
void morton_decode(int c, int* x, int* y) {
   *x = compact(c);
   *y = compact(c >> 1);
}

// Transpose a square matrix with dimensions that are even powers of two
// accessing the elements in Z-order.
void z_transpose(int I[2], int J[2], int D[m][n], int E[n][m]) {
   // printf("%i %i\n", 1 + I[1] - I[0], 1 + J[1] - J[0]);
   /*
   for (int i = I[0]; i <= I[1]; ++i)
      for (int j = J[0]; j <= J[1]; ++j) E[j][i] = D[i][j];
   */
   int num_elements = (1 + I[1] - I[0]) * (1 + J[1] - J[0]);
   for (int z = 0; z < num_elements; ++z) {
      int i, j;
      morton_decode(z, &i, &j);
      i += I[0];
      j += J[0];
      // printf("z: %i, (i,j): (%i, %i)\n", z, i, j);
      E[i][j] = D[j][i];
   }
}

void transpose(int I[2], int J[2], int D[m][n], int E[n][m]) {
   int num_rows = 1 + I[1] - I[0];
   int num_cols = 1 + J[1] - J[0];
   // printf("%i %i\n", num_rows, num_cols);
   /*
   if (is_power_of_two(num_rows) && is_power_of_two(num_cols)) {
      // The submatrix has dimensions that are even powers of two.
      // Handle the rest in square blocks.
      if (num_cols <= num_rows) {
         for (int i = I[0]; i <= I[1]; i += num_cols) {
            z_transpose((int[2]){i, i + num_cols - 1}, J, D, E);
         }
      } else {
         for (int j = J[0]; j <= J[1]; j += num_rows) {
            z_transpose(I, (int[2]){j, j + num_rows - 1}, D, E);
         }
      }
   */
   if (is_power_of_two(num_rows) && num_rows == num_cols) {
      // The submatrix is square with dimensions that are powers of two.
      z_transpose(I, J, D, E);
   } else if (num_cols <= num_rows) {
      // Horizontally slice D into two submatrices and recurse.
      int slice_height = isolate_rightmost_bit(num_rows);
      if (slice_height == num_rows) slice_height /= 2;
      // printf("slice_height: %i\n", slice_height);
      transpose((int[2]){I[0], I[0] + slice_height - 1}, J, D, E);
      transpose((int[2]){I[0] + slice_height, I[1]}, J, D, E);
   } else {
      // Vertically slice D into two submatrices and recurse.
      int slice_width = isolate_rightmost_bit(num_cols);
      if (slice_width == num_cols) slice_width /= 2;
      // printf("slice_width: %i\n", slice_width);
      transpose(I, (int[2]){J[0], J[0] + slice_width - 1}, D, E);
      transpose(I, (int[2]){J[0] + slice_width, J[1]}, D, E);
   }
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
