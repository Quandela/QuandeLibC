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

#ifndef FS_MAP_H
#define FS_MAP_H

#include <complex>
#include <iostream>

#include "fs_array.h"

/**
 * A Fock State map (fs-map) is used to map states with k-1 photons to states with k photons
 */
class fs_map {
    public:
        /**
         * Current version number used for serialization of the structure
         */
        static unsigned char version;
        /**
         * Build a fs-map given current and parent fs-array
         * @param fsa_current the fs-array for the current layer (k photons)
         * @param fsa_parent the fs-array of the parent layer (k-1 photons)
         * @param generate if true, immediately generates the structure in memory
         * @throws std::out_of_range if it is not possible to find parent state associated to current state
         */
        fs_map(const fs_array &fsa_current, const fs_array &fsa_parent, bool generate=false);
        /**
         * Delete a fs-map
         */
        ~fs_map();
        /**
         * number of parent state in the fs-map - is equivalent to `fsa_parent.count()`
         * @return the number of states
         */
        inline unsigned long long count() const { return _count; };
        /**
         * size of the parent map in memory - is `count()*_m*step` where step is the number of bytes necessary
         * to encode the index of a state in current layer
         * @return the number of states
         */
        inline unsigned long long size() const { return _count * _m * _step; };
        inline int get_m() const { return _m; };
        inline int get_n() const { return _n; };
        inline unsigned long long get_nc(unsigned long long idx, int m) const {
            unsigned char *ptr_pointer = _buffer + (idx * _m + m) * _step;
            int size_pointer = _step;
            unsigned long long idx_p1 = 0;
            bool all_xff = true;
            while (size_pointer--) {
                all_xff &= ptr_pointer[size_pointer] == 0xff;
                idx_p1 = (idx_p1 << 8) + (ptr_pointer[size_pointer]);
            }
            if (all_xff)
                return fs_npos;
            return idx_p1;
        }
        unsigned long long get(unsigned long long idx, int m) const;
        void generate() const;

        void compute_slos_layer(const std::complex<double> *p_u,
                                int m,
                                int mk,
                                std::complex<double> *p_coefs, unsigned long n_coefs,
                                const std::complex<double> *p_parent_coefs, unsigned long n_parent_coefs) const;

    private:
        int _step;
        unsigned long long _count;
        int _m;
        int _n;
        mutable unsigned char *_buffer;
        const fs_array *_pfsa_current;
        const fs_array *_pfsa_parent;
};

#endif
