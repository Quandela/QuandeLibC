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

#ifndef _PERMANENT_GLYNN_HPP
#define _PERMANENT_GLYNN_HPP

#include "optmul.h"
#include <cstdlib>
#include <cstring>

#include "memory_tools.h"

template<typename T>
T permanent_glynn(const T *A, int n) {
    if (A == nullptr) throw std::invalid_argument("A is null");
    if (n == 1) return A[0];

    T *rowsum;
    CHECK_MEMALIGN(posix_memalign((void **) &rowsum, 32, n * sizeof(T)));
    std::memset((void *) rowsum, 0, n * sizeof(T));

    for (int i = 0, base = 0; i < n; i++, base += n) {
        rowsum[i] = A[base];
        for (int k = 1; k < n; k++) rowsum[i] += A[base + k];
        rowsum[i] /= 2;
    }
    T sum = multiply_row<T>(rowsum, n);

    /* Loopless Gray binary Generation - Knuth Algorithm L */
    unsigned char *chi = (unsigned char *) malloc(n);
    std::memset(chi, 1, n);
    int *f;
    CHECK_MEMALIGN(posix_memalign((void **) &f, 32, n * sizeof(int)));
    for (int i = 0; i < n; i++) f[i] = i;

    int j = 0;
    while (j < n - 1) {
        if (chi[j]) {
            for (int i = 0, base = j; i < n; i++, base += n) rowsum[i] -= A[base];
            chi[j] = 0;
        } else {
            for (int i = 0, base = j; i < n; i++, base += n) rowsum[i] += A[base];
            chi[j] = 1;
        }
        if (j > 0) {
            sum += multiply_row<T>(rowsum, n);
            int k = j + 1;
            f[j] = f[k];
            f[k] = k;
            j = 0;
        } else {
            sum -= multiply_row<T>(rowsum, n);
            j = f[1];
            f[1] = 1;
        }
    }
    posix_memfree(f);
    free(chi);
    posix_memfree(rowsum);
    return 2. * sum;
}

#endif
