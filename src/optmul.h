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

#ifndef _OPTMUL_HPP
#define _OPTMUL_HPP

template<typename T>
T multiply_row(T* A, int n)
{
    T rowsumprod = A[0];
    for(int m=1; m < n; m++)
        rowsumprod *= A[m];
    return rowsumprod;
}

#ifdef __AVX__
#include <immintrin.h>
template<>
std::complex<double> multiply_row<std::complex<double>>(std::complex<double>* A, int n)
{
    if (n==1) return A[0];
    // pair multiplication of complex numbers from 0 to n
    // c1 c2 c3 c4 c5 .... cn
    // c1*c3 c2*c4 c5 .... cn
    // c1*c3*c5 c2*c4*c6 ... cn
    // ...
    // (c1*c3*c5*...*c(2k+1))*(c2*c4*c6*...*c(2k)) [*cn]
    static __m256d neg = _mm256_setr_pd(1.0, -1.0, 1.0, -1.0);

    __m256d C1 = _mm256_load_pd(reinterpret_cast<double*>(A));
    __m256d C3, C4, C5; // auxiliary for the calculation
    for (int m=2; m <= n-2; m+=2) {
        const __m256d &C2 = *reinterpret_cast<__m256d*>(A+m);
        C3 = _mm256_mul_pd(C1, C2);
        C4 = _mm256_permute_pd(C2, 0x5);
        C4 = _mm256_mul_pd(C4, neg);
        C5 = _mm256_mul_pd(C1, C4);
        C1 = _mm256_hsub_pd(C3, C5);
    }

    std::complex<double> *B=reinterpret_cast<std::complex<double>*>(&C1);
    std::complex<double> rowsumprod = B[0]*B[1];
    if (n%2) rowsumprod *= A[n-1];
    return rowsumprod;
}

template<>
double multiply_row<double>(double* A, int n)
{
    double rowsumprod=1;
    int lastidx=0;
    if (n>=4) {
        __m256d C1 = _mm256_load_pd(A);
        for (lastidx=4; lastidx <= n-4; lastidx+=4) {
            const __m256d &C2 = *reinterpret_cast<__m256d*>(A+lastidx);
            C1 = _mm256_mul_pd(C1, C2);
        }
        double *B=reinterpret_cast<double*>(&C1);
        __m128d C3=_mm_mul_pd (*reinterpret_cast<__m128d*>(B), *reinterpret_cast<__m128d*>(B+2));
        double *C=reinterpret_cast<double*>(&C3);
        rowsumprod = C[0]*C[1];
    }
    /* we still have up to 7 numbers - we can optimize a little bit more the final calculation */
    for(;lastidx<n; lastidx++) { rowsumprod *= A[lastidx];}
    return rowsumprod;
}

#endif // __AVX__

#endif
