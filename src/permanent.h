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

#ifndef _PERMANENT_HPP
#define _PERMANENT_HPP

#include "permanent_ryser.h"
#include "permanent_glynn.h"
#include <type_traits>
#include <thread>

template<typename T>
T permanent(const T* A, int n, int nthreads = 0, const std::string &ptype = "") {
    if (A == nullptr) throw std::invalid_argument("A is null");

    /* cannot use glynn for int (need to adapt the 2 divider) */
    if (ptype == "glynn" || (ptype.size() == 0 && (nthreads == 1 || nthreads == 2))) {
        if (std::is_same<T, long long>::value)
            throw (std::invalid_argument("cannot use glynn for int"));
        return permanent_glynn(A, n);
    }

    if (nthreads == 0)
        nthreads = std::thread::hardware_concurrency();

    return permanent_ryser(A, n, nthreads);
}

#endif
