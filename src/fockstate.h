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

#ifndef FOCKSTATE_H
#define FOCKSTATE_H

#include <vector>
#include <string>
#include <stdexcept>
#include <list>
#include <unordered_map>

#include "annotation.h"

typedef std::unordered_map<size_t, std::list<std::pair<int, annotation*>>> map_m_lannot;

class fockstate {
    friend class fs_array;

    public:
        fockstate();
        /* vacuum state */
        explicit fockstate(int m);
        /* construct from text representation |...〉, |...>, or [...] */
        explicit fockstate(const char *str);
        /* construct from text representation and possible annotation list */
        explicit fockstate(const char *str, const std::map<int, std::list<std::string>> &annotations);
        explicit fockstate(const std::vector<int> &fs_vec);
        explicit fockstate(const std::vector<int> &fs_vec, const std::map<int, std::list<std::string>> &annotations);
        fockstate(const fockstate &);
        fockstate(int m, int n);
        fockstate(int m, int n, const char *code, bool owned_data=false);
        fockstate(int m, int n, const char *code, map_m_lannot annots, bool owned_data=false);
        ~fockstate();
        fockstate copy() const;
        unsigned long long hash() const;
        fockstate &operator=(const fockstate &);

        /* operations on fockstate */
        /** iterator on fockstates */
        fockstate &operator+=(int);
        fockstate operator+(int) const;
        fockstate &operator++();
        /** tensor product **/
        fockstate operator*(const fockstate &) const;
        bool operator==(const fockstate &) const;
        bool operator!=(const fockstate &) const;

        /** access photon and mode **/
        int operator[](int idx) const;
        /** photon_idx to mode **/
        inline int photon2mode(int photon_idx) const {
            if (photon_idx < 0 || photon_idx >= _n) throw std::out_of_range("photon index out of range");
            return _code[photon_idx]-'A';
        }
        /** retrieve first photon idx in given mode - or -1 if none **/
        inline int mode2photon(int mode_idx) const {
            if (mode_idx < 0 || mode_idx >= _m) throw std::out_of_range("mode index out of range");
            int k=0;
            for(; k < _n && _code[k]-'A' < mode_idx; k++);
            if (k == _n || _code[k]-'A' != mode_idx) return -1;
            return k;
        }

        /** annotation specific functions **/
        bool has_annotations() const { return !_annotation_map.empty(); }
        bool has_polarization() const;
        void clear_annotations();
        std::list<annotation> get_mode_annotations(int) const;
        annotation get_photon_annotation(int idx) const;

        void set_mode_annotations(int, const std::list<annotation> &);

        /**
         * from annotated states distinguishable groups of non annotated indistinguishables fockstates
         * @return list of non annotated fockstates
         */
        std::list<fockstate> separate_state() const;

        /** slicing utilities **/
        fockstate slice(int start, int end, int step=1) const;
        fockstate set_slice(const fockstate &fs, int start, int end) const;

        /**
         * product of factorial $n_k$ used as normalization
         * @return the product of factorial
         */
        unsigned long long prodnfact() const;
        std::string to_str(bool show_annotations=true) const;
        inline int get_m() const { return _m; }
        inline int get_n() const { return _n; }
        inline const char *get_code() const { return _code; }
        void to_vect(std::vector<int> &) const;
        std::vector<int> to_vect() const;
        inline static unsigned long long hash_function(const char *s, int size=-1) {
            unsigned long long hash = 5381;
            for(int i=0; (size>=0 && i<size) || (size<0 && s[i]); i++)
                hash = ((hash << 5) + hash) + s[i];
            return hash;
        }
        class const_iterator
        {
            public:
                typedef const_iterator self_type;
                const_iterator(const fockstate *pfs, int mk) : _pfs(pfs), _mk(mk) { }
                self_type operator++() { _mk++; return *this; }
                int operator*() { return (*_pfs)[_mk]; }
                bool operator==(const self_type& rhs) { return _pfs == rhs._pfs && _mk == rhs._mk; }
                bool operator!=(const self_type& rhs) { return _pfs != rhs._pfs || _mk != rhs._mk; }
            private:
                const fockstate *_pfs;
                int _mk;
        };
        const_iterator begin() const { return {this, 0}; }
        const_iterator end() const { return {this, _m}; }
    private:
        void _check_slice(int &start, int &end, int step, int &slice_m, int &slice_n) const;
        int _m;
        int _n;
        char *_code;
        bool _owned_data;
        /* annotations of photons in different modes */
        map_m_lannot _annotation_map;

        void _parse_str(const char *str);
        void _set_annotations(const std::map<int, std::list<std::string>> &annotations);
        void _set_fs_vect(const std::vector<int> &fs_vect);
};

#endif
