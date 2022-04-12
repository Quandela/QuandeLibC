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

#include <iostream>
#include <limits>
#include <complex>
#include <vector>
#include "permanent.h"

using namespace std;
using ms = chrono::microseconds;
typedef std::numeric_limits< double > dbl;

void my_main(int argc, const char** argv) {
    // automatic detect number of threads
    int n_threads, n_iter;
    if (argc != 4)
        throw(invalid_argument("should have 4 arguments: n_threads n_iter algorithm"));
    if (sscanf(argv[1], "%d", &n_threads) != 1)
        throw(invalid_argument("cannot parse #threads"));
    if (sscanf(argv[2], "%d", &n_iter) != 1)
        throw(invalid_argument("cannot parse #iter"));
    if (strcmp(argv[3], "ryser") && strcmp(argv[3], "glynn"))
        throw(invalid_argument("unknown algorithm"));
    if (n_threads == 0)
        n_threads = thread::hardware_concurrency();
#if defined(P_COMPLEX)
    vector<complex<double>> input;
    complex<double> value;
#elif defined(P_INT)
    vector<long long> input;
    long long value;
#else
    vector<double> input;
    double value;
#endif
    std::string s;
    while (cin >> s) {
        double real;
#if defined(P_COMPLEX)
        double imaginary;
        if (sscanf(s.c_str(), "(%lf+%lfj)", &real, &imaginary) != 2) {
            if (sscanf(s.c_str(), "(%lf-%lfj)", &real, &imaginary) == 2) {
                imaginary = -imaginary;
            }
            else if (sscanf(s.c_str(), "%lf", &real) == 1)
                imaginary = 0;
            else
                throw(invalid_argument("incorrect complex number: "+s));
        }
        value = complex<double>(real, imaginary);
#else
        if (sscanf(s.c_str(), "%lf", &real) != 1)
            throw(invalid_argument("incorrect float number: "+s));
        value = real;
#endif
        input.push_back(value);
    }

    size_t n = sqrt(input.size());
    if (input.size() != n*n)
        throw(logic_error("Read " + to_string(input.size()) +
                          " elements which does not make a square matrix"));
    cerr << "dimension: " << n << "x" << n << endl;
    cerr << "running on " << n_threads << " threads" << " - " << n_iter << " iterations" << endl;
    long elapsed = 100000000;
    auto result = input[0];

    for(int i=0; i<n_iter+1; i++) {
        auto start = chrono::steady_clock::now();
        if (!strcmp(argv[3], "ryser"))
            result = permanent_ryser(input.data(), n, n_threads);
        else if (!strcmp(argv[3], "glynn"))
            result = permanent_glynn(input.data(), n);
        auto end = chrono::steady_clock::now();
        if (i)
            if (chrono::duration_cast<ms>(end - start).count() < elapsed)
                elapsed = chrono::duration_cast<ms>(end - start).count();
    }
    cout << "result:" << result << " elapsed by iter: "<< elapsed << " micro-seconds"<<endl;
}

int main(int argc, const char** argv) {
    cout.precision(dbl::max_digits10);
    try {
        my_main(argc, argv);
    } catch(exception& e) {
        cerr << "Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
