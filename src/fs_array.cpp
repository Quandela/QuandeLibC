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

#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "fs_array.h"

#define DEFAULT_FILENAME "layer-m%d-n%d.fsa"
#define BUFFER_LENGTH 30

const unsigned long long fs_npos = 0xffffffff;

void fs_array::_count_fs() {
    if (_p_mask) {
        fockstate fs(_m, _n);
        _count = 0;
        while(true) {
            if (_p_mask->match(fs)) _count++;
            if (!(++fs)._code) break;
        }
    } else {
        _count = 1;
        for (int nk = 1; nk <= _n; nk++) {
            _count = (_count * (nk + _m - 1)) / nk;
        }
    }
}

fs_array::fs_array(int m, int n): _buffer(nullptr), _m(m), _n(n), _count(0), _p_mask(nullptr) {
    _count_fs();
}

fs_array::fs_array(int m, int n, const fs_mask &mask): _buffer(nullptr),
                                                       _m(m),
                                                       _n(n),
                                                       _count(0),
                                                       _p_mask(new fs_mask(mask)) {
    _count_fs();
}

const unsigned char fs_array::version = 2;

fs_array::~fs_array() {
    delete [] _buffer;
}

unsigned long long fs_array::count() const {
    return _count;
}

unsigned long long fs_array::size() const {
    return _count*_n;
}

void fs_array::generate() const {
    if (_buffer)
        return;
    _buffer = new char[size()==0?1:size()];
    fockstate fs(_m, _n);
    unsigned long long idx=0;
    while(true) {
        int i;
        if (!_p_mask || _p_mask->match(fs)) {
            for(i=0;i<_n;i++) _buffer[i+idx] = fs._code[i];
            idx += _n;
        }
        if (!(++fs)._code) break;
    }
}

fs_array::const_iterator fs_array::find(const fockstate &fs) const {
    unsigned long long idx = find_idx(fs);
    if (idx == fs_npos) return end();
    return {this, idx};
}

unsigned long long fs_array::find_idx(const fockstate &fs) const {
    generate();
    if (fs.get_m() != _m)
        throw std::invalid_argument("incorrect fock state");
    // empty state
    if (!_n) {
        if (fs.get_n() == 0)
            return 0;
        else
            return fs_npos;
    }
    if (fs.get_n() != _n)
        return fs_npos;
    // binary search -> O(log_2 _count)
    char *code = fs._code;
    unsigned long long begin_range = 0;
    unsigned long long end_range = _count;
    unsigned long long middle;
    unsigned long long last_tested_idx = fs_npos;
    while ((end_range-begin_range)>1) {
        middle = (begin_range+end_range)>>1;
        int comparator = strncmp(code, _buffer+_n*middle, _n);
        if (comparator == 0) return middle;
        last_tested_idx = middle;
        if (comparator < 0) end_range = middle;
        else begin_range = middle;
    }
    middle = (begin_range+end_range)>>1;
    if (last_tested_idx != middle && strncmp(code, _buffer+_n*middle, _n) == 0)
        return middle;
    return fs_npos;
}

fockstate fs_array::operator[](unsigned long long idx) const {
    if (idx>=_count)
        throw std::out_of_range("index too large");
    generate();
    return {_m, _n, _buffer+idx*_n};
}

fs_array::const_iterator::const_iterator(const fs_array *fsa, bool first):_fsa(fsa),_pfs(nullptr) {
    if (first)
        idx = 0;
    else
        idx = fsa->_count;
    /* if fsa is not generated - just go through the different states */
    if (!fsa->_buffer) {
        _pfs = new fockstate(fsa->_m, fsa->_n);
        _find_next();
    }
}

fs_array::const_iterator::const_iterator(const fs_array *fsa, unsigned long long f_idx):_fsa(fsa),
                                                                                        _pfs(nullptr),
                                                                                        idx(f_idx) {
    if (!fsa->_buffer) {
        _pfs = new fockstate(fsa->_m, fsa->_n);
        _find_next();
        while(f_idx && _pfs->_code) {
            ++(*_pfs);
            _find_next();
            --f_idx;
        }
    }
}

fs_array::const_iterator::const_iterator(const_iterator &it):_fsa(it._fsa), idx(it.idx) {
    if (it._pfs)
        _pfs = new fockstate(*it._pfs);
    else
        _pfs = nullptr;
}

fs_array::const_iterator::const_iterator(const_iterator &&it) noexcept :_fsa(it._fsa), idx(it.idx) {
    _pfs = it._pfs;
    it._pfs = nullptr;
}

fs_array::const_iterator &fs_array::const_iterator::operator=(fs_array::const_iterator const&it) {
    if (&it == this)
        return *this;
    _fsa = it._fsa;
    delete _pfs;
    if (it._pfs)
        _pfs = new fockstate(*it._pfs);
    else
        _pfs = nullptr;
    idx = it.idx;
    return *this;
}

fs_array::const_iterator &fs_array::const_iterator::operator=(fs_array::const_iterator &&it) noexcept {
    _fsa = it._fsa;
    delete _pfs;
    _pfs = it._pfs;
    it._pfs = nullptr;
    idx = it.idx;
    return *this;
}

fs_array::const_iterator::~const_iterator() {
    delete _pfs;
}

void fs_array::const_iterator::_find_next() {
    if (_pfs) {
        while(_pfs->_code && _fsa->_p_mask && !_fsa->_p_mask->match(*_pfs)) {
            ++(*_pfs);
        }
    }
}

fs_array::const_iterator::self_type &fs_array::const_iterator::operator++() {
    if (idx<_fsa->_count) {
        ++idx;
        if (_pfs) { ++(*_pfs); _find_next(); }
    }
    return *this;
}

fockstate fs_array::const_iterator::operator*() {
    if (_pfs)
        return _pfs->copy();
    return {_fsa->_m, _fsa->_n, _fsa->_buffer+idx*_fsa->_n, false};
}

bool fs_array::const_iterator::operator==(const fs_array::const_iterator::self_type& rhs) const {
    return this->_fsa == rhs._fsa && this->idx == rhs.idx;
}

bool fs_array::const_iterator::operator!=(const fs_array::const_iterator::self_type& rhs) const {
    return this->idx != rhs.idx || this->_fsa != rhs._fsa;
}

void fs_array::norm_coefs(std::complex<double> *p_coefs) const {
    generate();
    const char *_code = _buffer;
    std::unordered_map<unsigned long, double> sqrt_o;
    for(unsigned long i=0; i < count(); i++, _code+=_n) {
        unsigned long p = 1;
        for(int j=0; j<_n;) {
            int k=1;
            while (j+k<_n && _code[j+k] == _code[j])
                p *= ++k;
            j += k;
        }
        auto it = sqrt_o.find(p);
        double coef;
        if (it == sqrt_o.end()) {
            coef = sqrt((double) p);
            sqrt_o[p] = coef;
        } else
            coef = it->second;
        p_coefs[i] *= coef;
    }
}
