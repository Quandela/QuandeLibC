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

#ifndef _PERMANENT_RYSER_HPP
#define _PERMANENT_RYSER_HPP

#include <cmath>
#include <thread>
#include <future>
#include <cstdlib>

#include "memory_tools.h"
#include "optmul.h"

// initially, inspired from: https://www.codeproject.com/Articles/21282/Compute-Permanent-of-a-Matrix-with-Ryser-s-Algorit
// misc optimization
// transformed dec2binarr into dec2idxarr
// replaced pow(-1, k) => k%2?-1:1 logic
// templated to all double and float
// introduce graycode ordering, update previous idx calculation only with graycode difference
// introduce complex number
// avx optimization for double and complex double numbers
// thread parallelization
// misc additional optimization, avoid test in loop

static inline int dec2idxarr(int *chi, int &diff, uint64_t k, int prev_size_set) {
    /* if prev_size_set is not null, then it is not the first item, in such case, we do not calculate the full
       sequence of idx corresponding to `k`, but the only difference between the previous chi and the new_chi
       we return this difference as `diff` parameter either positive or negative corresponding to row to add/remove
       in any case, chi is updated to become the new chi */
    int pos = 0;
    int idx = 0;
    diff = 0;
    if (prev_size_set) {
        while (k > 0) {
            if (k % 2) {
                if (idx == prev_size_set || chi[idx] != pos) {
                    diff = (pos + 1);
                    for (int i = prev_size_set; i > idx; i--) chi[i] = chi[i - 1];
                    chi[idx] = pos;
                    return prev_size_set + 1;
                }
                idx++;
            } else if (idx < prev_size_set && chi[idx] == pos) {
                diff = -(pos + 1);
                for (; idx < prev_size_set - 1; idx++) chi[idx] = chi[idx + 1];
                return prev_size_set - 1;
            }
            k = k >> 1;
            pos++;
        }
    } else {
        while (k > 0) {
            if (k % 2)
                chi[idx++] = pos;
            k = k >> 1;
            pos++;
        }
    }
    return idx;
}

template<typename T>
T permanent_ryser_block(const T *A, uint64_t from, uint64_t to, int n) // expects n by n matrix encoded as vector
{
    T sum = 0;
    int *chi;
    T *rowsum_arr;
    CHECK_MEMALIGN(posix_memalign((void **) &chi, 32, n * sizeof(int)));
    CHECK_MEMALIGN(posix_memalign((void **) &rowsum_arr, 32, n * sizeof(T)));

    int prev_size_set = 0;
    // loop all submatrices of A from graycode(`from` to `to`)
    for (uint64_t k = from; k < to; k++) {
        uint64_t new_graycode = (k ^ (k >> 1));
        int diff;
        int size_set = dec2idxarr(chi, diff, new_graycode, prev_size_set); // idx vector
        prev_size_set = size_set;

        // if we have just one diff with previous line, we can just compute the difference with previous line
        // this is why we are using graycode for...
        // unroll the different cases to avoid testing diff sign and calculating (m*n + diff>0?diff:-diff)-1
        // otherwise compute the full array
        if (diff > 0)
            for (int m = 0, base = diff - 1; m < n; m++, base += n) rowsum_arr[m] += A[base];
        else if (diff < 0)
            for (int m = 0, base = -diff - 1; m < n; m++, base += n) rowsum_arr[m] -= A[base];
        else {
            // no need to optimize the following, it happens only once per block
            for (int m = 0; m < n; m++) {
                rowsum_arr[m] = 0;
                for (int p = 0; p < size_set; p++)
                    rowsum_arr[m] += A[m * n + chi[p]];
            }
        }

        /* For complex numbers - we are spending ~2/3 of the time in the following */
        T rowsumprod = multiply_row<T>(rowsum_arr, n);

        // still some optimization here - as for glynn, we can correlate sign of diff
        // and parity of (n-size_set)%2 - meaning that we don't need the size_set
        if ((n - size_set) % 2)
            sum -= rowsumprod;
        else
            sum += rowsumprod;
    }

    posix_memfree(chi);
    posix_memfree(rowsum_arr);

    return sum;
}


template<typename T>
T permanent_ryser(const T *A, int n, int nthreads = 0) // expects n by n matrix encoded as vector
{
    if (A == nullptr) throw std::invalid_argument("A is null");
    uint64_t C = 1l << n;

    T result = T();
    std::vector<std::future<T>> results;
    uint64_t start = 1;
    uint64_t block_size = C / nthreads;

    for (auto i = 0; i < nthreads; ++i) {
        uint64_t end = (i == nthreads - 1) ? C : block_size * (i + 1);
        results.emplace_back(std::async(std::launch::async, permanent_ryser_block<T>, A, start, end, n));
        start = end;
    }
    for (auto &r: results)
        result += r.get();

    return result;
}

#endif
