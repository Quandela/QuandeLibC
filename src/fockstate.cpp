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
#include <cctype>
#include <algorithm>
#include <sstream>
#include <map>
#include "fockstate.h"

/* one-byte memory space that is used as pointer to 0-size fockstate buffer */
static char n0_buffer[1];

const char * skip_blanks(const char * str) {
    while (*str == ' ') str++;
    return str;
}

fockstate::fockstate(): _m(0), _n(0), _code(nullptr), _owned_data(true) {
}

fockstate::fockstate(int m): _m(m), _n(0), _code(n0_buffer), _owned_data(false) {
}

fockstate::fockstate(const char *str) {
    str = skip_blanks(str);
    char open_char = *str;
    if (*str != '[' && *str != '|' && *str != '(')
        throw std::invalid_argument("invalid fock state representation");
    str += 1;
    std::vector<int> fs_vect;
    _n = 0;
    _m = 0;
    while (true) {
        str = skip_blanks(str);
        if (!*str || !strchr("0123456789,{", *str) ||
            (!fs_vect.empty() && *str != ',') || (fs_vect.empty() && *str == ',')) break;
        if (*str == ',') {
            str = skip_blanks(++str);
        }
        int total_cn = 0;
        std::map<std::string, std::pair<int, annotation*>> count_annot_map;
        while (std::isdigit(*str) || *str == '{') {
            annotation *pa = nullptr;
            int cn = 0;
            if (*str == '{') {
                cn = 1;
            } else
                while (std::isdigit(*str)) {
                    cn = 10 * cn + (*str - '0');
                    str++;
                }
            if (*str == '{') {
                unsigned int j = 1;
                for (; str[j] && str[j] != '}'; j++);
                if (!str[j])
                    throw std::invalid_argument("invalid fock state representation (no annotation close)");
                std::string sa(str + 1, j - 1);
                pa = new annotation(sa.c_str());
                str += j + 1;
            }
            if (pa) {
                std::string annot_str = pa->to_str();
                if (annot_str.empty()) {
                    delete pa;
                }
                else if (count_annot_map.find(annot_str) == count_annot_map.end())
                    count_annot_map[annot_str] = std::make_pair(cn, pa);
                else {
                    count_annot_map[annot_str].first += cn;
                    delete pa;
                }
            }
            for (int i = 0; i < cn; i++) {
                total_cn++;
            }
        }
        _n += total_cn;
        if (!count_annot_map.empty()) {
            if (_annotation_map.find(fs_vect.size()) == _annotation_map.end())
                _annotation_map[fs_vect.size()] = std::list<std::pair<int, annotation*>>();
            for (auto iter: count_annot_map) {
                auto annot = iter.first;
                auto pair_count_annot = iter.second;
                _annotation_map[fs_vect.size()].push_back(std::make_pair(pair_count_annot.first,
                                                                            pair_count_annot.second));
            }
        }
        fs_vect.push_back(total_cn);
    }
    if (fs_vect.empty() && *str==',') {
        _m = 1;
        while (true) {
            str = skip_blanks(str);
            if (*str != ',') break;
            _m += 1;
            str++;
        }
    }
    if ((open_char == '[' && *str != ']') ||
        (open_char == '(' && *str != ')') ||
        (open_char == '|' && !(*str == '>' || strncmp(str, "〉", 3) == 0)))
            throw std::invalid_argument("invalid fock state representation (bad close)");

    if (strchr(">)]", *str)) str++; else str+=3;
    str = skip_blanks(str);
    if (*str)
        throw std::invalid_argument("invalid fock state representation (extra chars)");
    if (_m) {
        _owned_data = false;
        _code = nullptr;
        return;
    }
    _m = fs_vect.size();
    if (_n) {
        _owned_data = true;
        _code = new char[_n];
        int k = 0;
        for (int i = 0; i < _m; i++)
            for (int j = 0; j < fs_vect[i]; j++)
                _code[k++] = char(i + 'A');
    } else {
        _owned_data = false;
        _code = n0_buffer;
    }
}

fockstate::fockstate(const fockstate &b):_m(b._m), _n(b._n) {
    if (b._code) {
        if (_n) {
            _code = new char[_n];
            memcpy(_code, b._code, _n);
            _owned_data = true;
        } else {
            _code = n0_buffer;
            _owned_data = false;
        }
        for(auto iter: b._annotation_map) {
            auto idx = iter.first;
            auto &la = iter.second;
            _annotation_map[idx] = std::list<std::pair<int, annotation*>>();
            for(auto p: la) {
                _annotation_map[idx].push_back(std::make_pair(p.first, new annotation(*p.second)));
            }
        }
    } else {
        _code = nullptr;
    }
}

fockstate &fockstate::operator=(const fockstate &b) {
    if (&b == this) return *this;
    if (_owned_data && _code) {
        delete [] _code;
    }
    clear_annotations();
    for(auto iter: b._annotation_map) {
        auto idx = iter.first;
        auto &la = iter.second;
        _annotation_map[idx] = std::list<std::pair<int, annotation*>>();
        for(auto const & p: la)
            _annotation_map[idx].push_back(std::make_pair(p.first, new annotation(*p.second)));
    }
    _n = b._n;
    _m = b._m;
    if (b._code) {
        if (_n) {
            _code = new char[_n];
            memcpy(_code, b._code, _n);
            _owned_data = true;
        } else {
            _code = n0_buffer;
            _owned_data = false;
        }
    } else
        _code = nullptr;

    return *this;
}

fockstate::fockstate(const std::vector<int> &fs_vect):_m(fs_vect.size()) {
    _n = 0;
    for(int i=0; i<_m; i++) _n += fs_vect[i];
    if (!_n) {
        _code = n0_buffer;
        _owned_data = false;
        return;
    }
    _code = new char[_n];
    _owned_data = true;
    int k = 0;
    for(int i=0; i<_m; i++)
        for(int j=0; j<fs_vect[i]; j++)
            _code[k++] = char(i+65);
}

fockstate::fockstate(int m, int n): _m(m), _n(n) {
    if (!_n) {
        _code = n0_buffer;
        _owned_data = false;
        return;
    }
    _code = new char[_n];
    _owned_data = true;
    ::memset(_code, 'A', _n);
}

fockstate::fockstate(int m, int n, const char *code, bool owned_data):_m(m), _n(n), _code((char*)code),
                                                                      _owned_data(owned_data) {
}

fockstate::fockstate(int m, int n, const char *code, const map_m_lannot &annots, bool owned_data):
                                                                      _m(m), _n(n), _code((char*)code),
                                                                      _owned_data(owned_data),
                                                                      _annotation_map(annots) {
}

fockstate::~fockstate() {
    if (_owned_data && _code)
        delete [] _code;
    clear_annotations();
}

fockstate fockstate::copy() const {
    return {*this};
}

std::vector<int> fockstate::to_vect() const {
    std::vector<int> fs_vect(_m);
    for(int i=0;i<_n; i++)
        fs_vect[_code[i]-65]++;
    return fs_vect;
}

void fockstate::to_vect(std::vector<int> &fs_vect) const {
    fs_vect.resize(_m);
    std::fill(fs_vect.begin(), fs_vect.end(), 0);
    for(int i=0;i<_n; i++)
        fs_vect[_code[i]-65]++;
}

fockstate fockstate::operator+(const fockstate &b) const {
    if (!_code || !b._code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    if (_m != b._m)
        throw std::invalid_argument("fockstate needs to have same dimension to add");
    if (b._n == 0) return *this;
    char *_new_code = new char[_n + b._n];
    int k_this=0;
    int k_b=0;
    int k=0;
    while (k_this<_n || k_b<b._n) {
        if (k_this==_n || (k_b != b._n && b._code[k_b] <= _code[k_this]))
            _new_code[k++] = b._code[k_b++];
        else
            _new_code[k++] = _code[k_this++];
    }
    map_m_lannot new_annotation_map;
    for(auto iter: _annotation_map) {
        auto idx = iter.first;
        auto &list_self_annots = iter.second;
        new_annotation_map[idx] =  std::list<std::pair<int, annotation*>>();
        for(auto p_toadd: list_self_annots) {
            new_annotation_map[idx].push_back(std::make_pair(p_toadd.first, new annotation(*p_toadd.second)));
        }
    }
    for(auto iter: b._annotation_map) {
        auto idx = iter.first;
        auto &list_b_annots = iter.second;
        if (new_annotation_map.find(idx) == new_annotation_map.end())
            new_annotation_map[idx] = std::list<std::pair<int, annotation*>>();
        for(auto p_toadd: list_b_annots) {
            std::string annot = p_toadd.second->to_str();
            bool found = false;
            for(auto & p_current:  new_annotation_map[idx]) {
                if (p_current.second->to_str() == annot) {
                    p_current.first += p_toadd.first;
                    found = true;
                    break;
                }
            }
            if (!found)
                new_annotation_map[idx].push_back(std::make_pair(p_toadd.first, new annotation(*p_toadd.second)));
        }
    }

    return {_m, k, _new_code, new_annotation_map, true};
}

fockstate fockstate::operator+(int c) const {
    if (!_code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    fockstate fs(*this);
    while (c) {
        ++fs;
        c--;
    }
    return fs;
}

fockstate &fockstate::operator++() {
    if (!_code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    int i;
    for(i=_n-1; i>=0 && _code[i]==_m-1+'A'; i--);
    if (i<0) {
        if (_owned_data)
            delete [] _code;
        _code = nullptr;
        return *this;
    }
    if (!_owned_data) {
        char *_new_code = new char[_n];
        memcpy(_new_code, _code, _n);
        _code = _new_code;
        _owned_data = true;
    }
    _code[i] += 1;
    for(int j=i+1; j<_n; j++)
        _code[j] = _code[i];
    return *this;
}

fockstate fockstate::operator*(const fockstate &b) const {
    if (!_code || !b._code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    char *_new_code = new char[_n + b._n];
    int k=0;
    while (k < _n) {
        _new_code[k] = _code[k];
        k++;
    }
    while (k < _n+b._n) {
        _new_code[k] = b._code[k-_n] + _m;
        k++;
    }
    map_m_lannot new_annotation_map;
    for(auto iter: _annotation_map) {
        auto idx = iter.first;
        auto list_self_annots = iter.second;
        new_annotation_map[idx] =  std::list<std::pair<int, annotation*>>();
        for(auto p_toadd: list_self_annots) {
            new_annotation_map[idx].push_back(std::make_pair(p_toadd.first, new annotation(*p_toadd.second)));
        }
    }
    for(auto iter: b._annotation_map) {
        auto idx = iter.first;
        auto list_b_annots = iter.second;
        new_annotation_map[idx+_m] = std::list<std::pair<int, annotation*>>();
        for(auto p_toadd: list_b_annots) {
            new_annotation_map[idx+_m].push_back(std::make_pair(p_toadd.first, new annotation(*p_toadd.second)));
        }
    }

    return {_m+b._m, k, _new_code, new_annotation_map, true};
}

std::list<annotation> fockstate::get_mode_annotations(unsigned int idx) const {
    std::list<annotation> l;
    auto map_iter = _annotation_map.find(idx);
    int i=0;
    if (map_iter != _annotation_map.end()) {
        for(auto const &p: map_iter->second) {
            for(int j=0; j<p.first; i++, j++) {
                l.push_back(annotation(*p.second));
            }
        }
    }
    for(; i<(*this)[idx]; i++)
        l.push_back(annotation());
    return l;
}

bool fockstate::has_polarization() const {
    if (_annotation_map.empty()) return false;
    for(auto iter: _annotation_map) {
        auto la = iter.second;
        for(auto const &p: la) {
            if (p.second->has_polarization()) return true;
        }
    }
    return false;
}

void fockstate::clear_annotations() {
    for(auto iter: _annotation_map) {
        auto la = iter.second;
        for(auto const & p: la)
            delete p.second;
    }
    _annotation_map.clear();
}

void fockstate::_check_slice(int &start, int &end, int step, int &slice_m, int &slice_n) const {
    if (start < 0)
        start += _m;
    if (end < 0)
        end += _m;
    if (start < 0) start = 0;
    if (end < 0) end = 0;
    if (end > _m) end = _m;
    if (!_code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    /* count photons in the slice */
    slice_m = 0;
    for(int i=start; i<end; i+=step)
        slice_m++;
    slice_n = 0;
    for(int i=0; i<_n; i++)
        if (_code[i] >= start+'A' && _code[i] < end+'A' && (step == 1 || (_code[i]-start-'A') % step == 0))
            slice_n++;
}

fockstate fockstate::slice(int start, int end, int step) const {
    int slice_m, slice_n;
    _check_slice(start, end, step, slice_m, slice_n);
    if (slice_n == 0)
        return {slice_m, 0};
    char *_new_code = new char[slice_n];
    for(int k=0, i=0; i<_n; i++)
        if (_code[i] >= start+'A' && _code[i] < end+'A' && (step == 1 || (_code[i]-start-'A') % step == 0)) {
            _new_code[k++] = (_code[i]-start-'A') / step + 'A';
        }
    map_m_lannot new_annotation_map;
    for(int j=0, i=start; i<end; i+=step, j++) {
        auto iter = _annotation_map.find(i);
        if (iter != _annotation_map.end()) {
            new_annotation_map[j] = std::list<std::pair<int, annotation*>>();
            for(auto const &p: iter->second)
                new_annotation_map[j].push_back(std::make_pair(p.first, new annotation(*p.second)));
        }
    }
    return {slice_m, slice_n, _new_code, new_annotation_map, true};
}

fockstate fockstate::set_slice(const fockstate &fs, int start, int end) const {
    int slice_m, slice_n;
    _check_slice(start, end, 1, slice_m, slice_n);
    if (slice_m != fs.get_m())
        throw std::invalid_argument("invalid fockstate to replace in slice");
    int new_n = get_n()-slice_n+fs.get_n();
    if (new_n == 0)
        return {get_m(), 0};
    char *_new_code = new char[new_n];
    int k = 0;
    int i = 0; /* iterator on current fockstate */
    // photons on lower mode
    for(; _code && i < _n && _code[i] < start+'A'; i++)
        _new_code[k++] = _code[i];
    // insert the slice photons
    for(int j=0; j < fs._n; j++)
        _new_code[k++] = fs._code[j]+start;
    for(;_code && i<_n && _code[i] < end+'A'; i++);
    // add photons on higher modes
    for(;_code && i<_n;i++)
        _new_code[k++] = _code[i];
    map_m_lannot new_annotation_map;
    for(auto iter: _annotation_map) {
        auto idx = iter.first;
        auto la = iter.second;
        if (int(idx) < start || int(idx) >= end) {
            new_annotation_map[idx] = std::list<std::pair<int, annotation *>>();
            for (auto p: la) {
                new_annotation_map[idx].push_back(std::make_pair(p.first, new annotation(*p.second)));
            }
        }
    }
    for(auto iter: fs._annotation_map) {
        auto idx = iter.first;
        auto lb = iter.second;
        new_annotation_map[idx+start] = std::list<std::pair<int, annotation *>>();
        for (auto p: lb) {
            new_annotation_map[idx+start].push_back(std::make_pair(p.first, new annotation(*p.second)));
        }
    }
    return {get_m(), new_n, _new_code, true};
}

fockstate &fockstate::operator+=(const fockstate &b) {
    if (!_code || !b._code)
        throw std::invalid_argument("cannot make operation on ndef-state");
    if (_m != b._m)
        throw std::invalid_argument("fockstate needs to have same dimension to add");
    if (b._n == 0) {
        return *this;
    }
    char *_new_code = new char[_n + b._n];
    int k_this=0;
    int k_b=0;
    int k=0;
    while (k_this < _n || k_b < b._n) {
        if (k_this == _n || (k_b != b._n && b._code[k_b] <= _code[k_this]))
            _new_code[k++] = b._code[k_b++];
        else
            _new_code[k++] = _code[k_this++];
    }
    for(auto iter: b._annotation_map) {
        auto idx = iter.first;
        auto list_b_annots = iter.second;
        if (_annotation_map.find(idx) == _annotation_map.end())
            _annotation_map[idx] = std::list<std::pair<int, annotation*>>();
        for(auto p_toadd: list_b_annots) {
            std::string annot = p_toadd.second->to_str();
            bool found = false;
            for(auto & p_current: _annotation_map[idx]) {
                if (p_current.second->to_str() == annot) {
                    p_current.first += p_toadd.first;
                    found = true;
                    break;
                }
            }
            if (!found)
                _annotation_map[idx].push_back(std::make_pair(p_toadd.first, new annotation(*p_toadd.second)));
        }
    }
    _n = k;
    if (_owned_data)
        delete [] _code;
    else
        _owned_data = true;
    _code = _new_code;
    return *this;
}

fockstate &fockstate::operator+=(int c) {
    while (c) {
        ++(*this);
        c--;
    }
    return *this;
}


unsigned long long fockstate::prodnfact() const {
    unsigned long long p = 1;
    for(int i=0; i<_n;) {
        int k=1;
        while (i+k<_n && _code[i+k] == _code[i])
            p *= ++k;
        i += k;
    }
    return p;
}

unsigned long long fockstate::hash() const {
    return hash_function(this->to_str().c_str());
}

bool fockstate::operator==(const fockstate &b) const {
    auto const &a = *this;
    if (a._m != b._m) return false;
    if (a._m == 0 && b._m == 0) return true;
    if (a._code == nullptr && b._code == nullptr) return true;
    if (a._code == nullptr || b._code == nullptr) return false;
    for (int i = 0; i < a._n; i++)
        if (a._code[i] != b._code[i]) return false;
    if (a._annotation_map.size() != b._annotation_map.size())
        return false;
    for (auto iter: a._annotation_map) {
        auto idx = iter.first;
        auto la = iter.second;
        auto ilb = b._annotation_map.find(idx);
        if (ilb == b._annotation_map.end())
            return false;
        for (auto const &pa: la) {
            bool found = false;
            for (auto const &pb: ilb->second) {
                if (pa.first == pb.first && pa.second->to_str() == pb.second->to_str()) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
    }
    return true;
}

bool fockstate::operator!=(const fockstate &b) const {
    return !(*this==b);
}

std::string fockstate::to_str(bool show_annotations) const {
    std::stringstream ss;
    ss << "|";
    if (_code) {
        std::vector<int> fs_vect(_m);
        std::vector<std::string> annots_vect(_m);
        for (int i = 0; i < _n; i++) {
            fs_vect[_code[i] - 'A']++;
        }
        if (show_annotations) {
            for (int i = 0; i < _m; i++) {
                const auto map_item = _annotation_map.find(i);
                if (map_item != _annotation_map.end()) {
                    for (auto const &p: map_item->second) {
                        std::stringstream s;
                        int count = p.first;
                        annotation *a = p.second;
                        if (count > 1) s << count;
                        annots_vect[i] += s.str() + "{" + a->to_str() + "}";
                        fs_vect[i] -= count;
                    }
                }
            }
        }
        for (int i = 0; i < _m; i++) {
            if (i) ss << ",";
            ss << annots_vect[i];
            if (annots_vect[i].empty() || fs_vect[i])
                ss << fs_vect[i];
        }
    } else {
        for (int i = 0; i < _m; i++) {
            if (i) ss << ",";
        }
    }
    ss << ">";
    return ss.str();
}

int fockstate::operator[](int idx) const {
    if (idx<0 || idx>=_m)
        throw std::out_of_range("invalid mode");
    int nc=0;
    for(int i=0;i<_n && _code[i]-'A'<=idx; i++)
        if (_code[i]-'A'==idx) nc++;
    return nc;
}
