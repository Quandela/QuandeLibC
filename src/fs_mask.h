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

#ifndef QUANDELIBC_FS_MASK_H
#define QUANDELIBC_FS_MASK_H

#include <list>
#include <string>

#include "fockstate.h"

/**
 * A Fock State mask (fs_mask) is used to constraint Fock space in fs_map and fs_array.
 * This is important to avoid combinatorial explosion of the memory structure when working
 * on identified subsets - for instance because of heralding conditions.
 * fs_mask are defined by list of inclusive conditions (OR) - each of them represented as a string C
 * of length m with following conventions:
 *   - C[i] cannot take ',' or \x00 value - these characters are use for constructor
 *   - C[i] == ' ' if there is no constraint on mode i
 *   - C[i] == [0x30-0x50] if there are exactly ord(C[i])-48 photon in mode i (up to 32 photons)
 *   - other codes are reserved for further usage
 *  A mask is defined for a given number of photons (n) - if the fockstate is not fully populated
 *  with n-photons then the mask can apply as long as the number of expected errors is not higher
 *  than the differences of photon count
 */
class fs_mask {
public:
    /**
     * Empty mask constructor
     *
     * @param m number of modes on which the mask apply
     * @param n number of photons required for the mask
     */
    explicit fs_mask(int m, int n);
    /**
     * mask constructor unique string
     *
     * @param m number of modes on which the mask apply
     * @param conditions a list of string conditions
     */
    fs_mask(int m, int n, const std::string &condition);
    /**
     * mask constructor from list of strings
     *
     * @param m number of modes on which the mask apply
     * @param conditions a list of string conditions
     */
    fs_mask(int m, int n, std::list<std::string> conditions);
    /**
     * copy constructor
     */
    fs_mask(const fs_mask &fs);
    /**
     * test if mask match a specific fockstate
     *
     * @param fs the (potentially incomplete) fockstate to match against
     * @param allow_missing allow missing photons
     * @return boolean result of the match
     */
    bool match(const fockstate &fs, bool allow_missing=true) const;
private:
    const int _m;
    const int _n;
    std::list<std::string> _conditions;
};

#endif //QUANDELIBC_FS_MASK_H
