// MIT License
//
// Copyright (c) 2022 Quandela
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _SUB_PERMANENTS_HPP
#define _SUB_PERMANENTS_HPP

/* from Clifford&Clifford 2017 paper (lemma 2) */

#include "memory_tools.h"

template<typename T>
void sub_permanents(const T* A, int n, T* p) {
  /* we expect A to be a (n+1) rows, (n) columns matrix, we will return n+1 permanents of
     the matrices excluding row j
     output in p which is supposed to be size m */
  int m = n + 1;
  if (n==1) { p[0] = A[1]; p[1] = A[0]; return; }

  T *rowsum, *q;
  CHECK_MEMALIGN(posix_memalign((void**)&rowsum, 32, m*sizeof(T)));

  for(int i=0, base=0; i<m; i++, base+=n) {
    rowsum[i] = A[base];
    for(int k=1; k<n; k++) rowsum[i] += A[base+k];
    rowsum[i] /= 2;
  }

  CHECK_MEMALIGN(posix_memalign((void**)&q, 32, m*sizeof(T)));
  T prev_value=1;
  for(int i=0; i<m; i++)
    prev_value = q[i] = prev_value*rowsum[i];

  p[m-1] = q[m-2];
  T t = rowsum[m-1];
  for(int i = m-2; i > 0; i--){
      p[i] = t*q[i-1];
      t *= rowsum[i];
  }
  p[0] = t;

  unsigned char *chi = (unsigned char*)malloc(n);
  std::memset(chi, 1, n);

  /* Loopless Gray binary Generation - Knuth Algorithm L */
  unsigned int* f = (unsigned int*)malloc(n*sizeof(unsigned int));
  for(int i=0; i<n; i++) f[i] = i;

  bool s = true;

  int j = 0;
  while (j < n-1) {
    for(int i=0, base=j; i<m; i++, base+=n) rowsum[i] += chi[j] ? -A[base] : A[base];
    chi[j] = 1-chi[j];
    prev_value = 1;
    for(int i=0; i<m; i++) prev_value = q[i] = prev_value*rowsum[i];
    if (s) { t = -rowsum[m-1]; p[m-1] -= q[m-2]; }
    else { t = rowsum[m-1]; p[m-1] += q[m-2]; }
    for(int i = m-2; i > 0; i--){
      p[i] += t*q[i-1];
      t *= rowsum[i];
    }
    p[0] += t;
    s = !s;
    if (j > 0) { f[j] = f[j+1]; f[j+1] = j+1; j = 0; }
    else { j = f[1]; f[1] = 1; }
  }
  free(f);
  free(chi);
  posix_memfree(q);
  posix_memfree(rowsum);
  for(int i=0; i<m; i++) p[i] = 2.*p[i];
}

#endif
