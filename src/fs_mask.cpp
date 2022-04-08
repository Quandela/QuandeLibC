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

#include "fs_mask.h"

fs_mask::fs_mask(int m, int n):_m(m),_n(n) {
}

fs_mask::fs_mask(int m, int n, const std::string &condition):_m(m), _n(n) {
    _conditions.push_back(condition);
}

fs_mask::fs_mask(int m, int n, std::list<std::string> conditions):_m(m), _n(n),
                                                                  _conditions(std::move(conditions)) {
}

fs_mask::fs_mask(const fs_mask &fs):_m(fs._m), _n(fs._n), _conditions(fs._conditions) {}

bool fs_mask::match(const fockstate &fs, bool allow_missing) const {
    /**
     * we match a fockstate if there are no conditions, or if one of the conditions match
     * if allow_missing is True, number of errors cannot exceed missing
     * in any case we do not allow extraneous photons in a mode
     * if allow_missing is False, no error is allowed
     **/
    if (_conditions.empty())
        return true;
    for(const std::string &c: _conditions) {
        int allowed_errors=0;
        if (allow_missing)
            allowed_errors = _n-fs.get_n();
        for(int i=0; allowed_errors>=0 && i<_m; i++) {
            if (c[i]>=0x30 && c[i]<0x50) {
                /* there cannot be extraneous photons */
                if (fs[i] > (c[i] - 0x30))
                    allowed_errors = -1;
                else
                    allowed_errors -= c[i] - 0x30 - fs[i];
            }
        }
        if (allowed_errors>=0) return true;
    }
    return false;
}
