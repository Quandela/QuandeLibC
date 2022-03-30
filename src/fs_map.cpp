/*
MIT License

Copyright (c) 2022 Quandela

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "fs_map.h"
#include "fockstate.h"

namespace fs = std::filesystem;

#define BUFFER_LENGTH 30

static fs::path get_dirfile(const char *fd_name, const char *template_name, int m, int n, int k) {
    fs::path fpath(fd_name);
    if (fs::is_directory(fpath)) {
        char f_name[BUFFER_LENGTH];
        snprintf(f_name, BUFFER_LENGTH, template_name, m, n, k);
        fpath /= f_name;
    }
    return fpath;
}

struct NStrHash {
    int _size;
    explicit NStrHash(int size):_size(size) {}
    unsigned long long operator()(const char *s) const {
        return fockstate::hash_function(s, _size);
    }
};

struct NStrCompare {
    int _size;
    explicit NStrCompare(int size):_size(size) {}
    bool operator()(const char *s1, const char *s2) const {
        return memcmp(s1, s2, _size) == 0;
    }
};

typedef std::unordered_map<const char*, unsigned long long, NStrHash, NStrCompare> NStrUMap;

unsigned char fs_map::version = 1;

/* given layer nk generate map between nk-1 and nk */
fs_map::fs_map(const fs_array &fsa_current, const fs_array &fsa_parent, bool do_generate):_buffer(nullptr),
                                                                                          _pfsa_current(&fsa_current),
                                                                                          _pfsa_parent(&fsa_parent) {
    int nk = fsa_current.get_n();
    _m = fsa_current.get_m();
    _n = nk-1;
    assert(fsa_parent.get_n()==_n);
    _step = 0;
    // the number of bytes necessary to encode an index in fsa_current
    // keep 0xff..ff for npos
    for(auto c=fsa_current._count+1; c > 0; _step++, c >>= 8);
    _count = fsa_parent._count;
    if (do_generate)
        generate();
}

fs_map::fs_map(const char *fd_name, int m, int n): _m(m), _n(n), _pfsa_current(nullptr), _pfsa_parent(nullptr) {
    fs::path fpath = get_dirfile(fd_name, "layer-m%d-n%d:%d.fsm", _m, _n, _n + 1);
    std::ifstream rf(fpath.c_str(), std::ios::binary);
    if (!rf)
        throw std::runtime_error("cannot open file");
    char buffer[4];
    rf.read(buffer, 4);
    if (std::string(buffer, 3) != "FSM" || buffer[3] != version)
        throw std::invalid_argument("incorrect FSA file version");
    unsigned char cm = _m;
    unsigned char cn = _n;
    rf.read((char *)&cm, 1);
    rf.read((char *)&cn, 1);
    if (m >= 0 && m != cm)
        throw std::invalid_argument("not right mode");
    _m = cm;
    if (n >= 0 && n != cn)
        throw std::invalid_argument("not right number of photons");
    _n = cn;
    _count = 1;
    for(int nk=1; nk<=n; nk++) {
        _count = (_count*(nk+m-1))/nk;
    }
    unsigned long long Mnp1 = _count*(_n+_m)/(_n+1);
    // TODO: fix incorrect _count for masked patterns
    _step = 0;
    for(auto c=Mnp1; c > 0; _step++, c >>= 8);
    _buffer = new unsigned char[size()];
    rf.read((char*)_buffer, size());
}

void fs_map::save(const char *fd_name) const {
    generate();
    fs::path fpath = get_dirfile(fd_name, "layer-m%d-n%d:%d.fsm", _m, _n, _n + 1);
    std::ofstream wf(fpath.c_str(), std::ios::out | std::ios::binary);
    if (!wf)
        throw std::runtime_error("cannot open file");
    wf.write("FSM", 3);
    wf.write((char*)&version, 1);
    unsigned char cm = _m;
    unsigned char cn = _n;
    wf.write((char*)&cm, 1);
    wf.write((char*)&cn, 1);
    wf.write((const char *)_buffer, size());
}

void fs_map::generate() const {
    if (_buffer) return;
    _pfsa_current->generate();
    _pfsa_parent->generate();
    int nk = _n+1;
    /* the map is an array of size _count (number of states in parent fsa) * m - each map cell is the transition
     * between parent fsa and current fsa when adding the additional photon in mode m */
    _buffer = new unsigned char[size()];
    ::memset(_buffer, 0xff, size());
    NStrUMap index_current_level(0, NStrHash(_n), NStrCompare(_n));
    index_current_level.reserve(2*_count);
    unsigned long long idx = 0;
    /* fsa array for level n (parent fsa) can be directly obtained from current fsa array - just skipping first photon
     * index current level is the address of parent state in parent fsa */
    for(fockstate fs: *_pfsa_parent) {
        index_current_level[fs.get_code()] = idx;
        idx++;
    }
    const char *state_nk = _pfsa_current->_buffer;
    char *fs_temp=new char[_n];
    /* simply go through the current state, and build all the possible parent states, get their index
     * and save them in the "map" */
    for(unsigned long long k=0; k<_pfsa_current->_count; k++, state_nk+=nk) {
        /* starting from state_k[i*nk] => builds the fock_state-1 with one photon less */
        int prev_i = 0;
        for(int i=0; i<nk; i++) {
            if (i<_n && state_nk[i+1] == state_nk[i])
                continue;
            for(int h=prev_i;h<i;h++)
                fs_temp[h] = state_nk[h];
            for(int h=i+1;h<nk;h++) 
                fs_temp[h-1] = state_nk[h];
            prev_i = i;
            /* search fs_temp in index of previous level */
            unsigned long long idx_m1;
            if (nk>1) 
                idx_m1 = index_current_level[fs_temp];
            else
                idx_m1 = 0;
            /* we save the pointer to current state (idx_current) in idx_m1 - mode state_nk[i] */
            unsigned char *ptr_pointer = _buffer+(idx_m1*_m+state_nk[i]-65)*_step;
            int size_pointer = _step;
            unsigned long long idx_current = k;
            while (size_pointer--) {
                *(ptr_pointer++) = idx_current & 0xFF;
                idx_current >>= 8;
            }
        }
    }
    delete [] fs_temp;
}

fs_map::~fs_map() {
    delete [] _buffer;
}

unsigned long long fs_map::get(unsigned long long idx, int m) {
    if (m>=_m)
        throw std::out_of_range("mode id too large");
    if (idx>=_count)
        throw std::out_of_range("idx too large");
    generate();
    unsigned char *ptr_pointer = _buffer + (idx*_m+m)*_step;
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
