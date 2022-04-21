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

#ifndef FS_ARRAY_H
#define FS_ARRAY_H

#include <cstring>
#include <complex>

#include "fockstate.h"
#include "fs_mask.h"
#include "memory_tools.h"

class fs_map;

extern const unsigned long long fs_npos;

class fs_array {
    /* fs_array are ordered vector of all possible fock states for given m, n */
    friend class fs_map;
    public:
        static const unsigned char version;
        fs_array(int m, int n);
        fs_array(int m, int n, const fs_mask &mask);
        ~fs_array();
        unsigned long long count() const;
        unsigned long long size() const;
        inline int get_m() const { return this->_m; }
        inline int get_n() const { return this->_n; }
        void generate() const;
        fockstate operator[](unsigned long long) const;
        class const_iterator
        {
            public:
                typedef const_iterator self_type;
                const_iterator(const fs_array *fsa, bool first);
                const_iterator(const fs_array *fsa, unsigned long long f_idx);
                const_iterator(const_iterator &i);
                const_iterator(const_iterator &&i) noexcept;
                ~const_iterator();
                const_iterator &operator=(const_iterator const&i);
                const_iterator &operator=(const_iterator &&i) noexcept;
                self_type &operator++();
                fockstate operator*();
                bool operator==(const self_type& rhs) const;
                bool operator!=(const self_type& rhs) const;
            private:
                void _find_next();
                const fs_array *_fsa;
                fockstate *_pfs;
            public:
                unsigned long long idx;
        };
        /**
         * find iterator of a fockstate
         * @param fs_vec the fockstate
         * @return the iterator, `end()` if not found
         */
        const_iterator find(const fockstate &fs_vec) const;
        /**
         * find index of a fockstate
         * @param fs_vec the fockstate
         * @return the idx of the fockstate or npos if not found
         */
        unsigned long long find_idx(const fockstate &fs_vec) const;
        const_iterator begin() const { return {this, true}; }
        const_iterator end() const { return {this, false}; }
        void norm_coefs(std::complex<double> *p_coefs) const;
    private:
        void _count_fs();
        mutable char *_buffer;
        int _m;
        int _n;
        unsigned long long _count;
        const fs_mask *_p_mask;
};

#endif
