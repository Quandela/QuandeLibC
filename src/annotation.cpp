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

#include <sstream>

#include "annotation.h"

static constexpr float pi = 3.14159265358979323846;

annotation::annotation(const char *str) {
    if (!*str) {
        return;
    }
    /* sequence of , separated annotation tag */
    do {
        /* annotation format is [A-Za-z0-9_]+:(\({DOUBLE},{DOUBLE}\)|{DOUBLE}|{COMPLEX}\) or P:[HVADLR] */
        unsigned int i = 0;
        for (; (str[i] >= 'A' && str[i] <= 'Z') ||
               (str[i] >= 'a' && str[i] <= 'z') ||
               (i > 0 && str[i] >= '0' && str[i] <= '9') ||
               (str[i] == '_'); i++) {}
        if (!str[i])
            throw std::invalid_argument("invalid annotation (no key-value separator)");
        if (str[i] != ':')
            throw std::invalid_argument("invalid annotation (invalid key format)");
        std::string _name = std::string(str, 0, i);
        int level = 0;
        int j;
        for(j=i+1; str[j] && !(level==0 && str[j]==','); j++)
        {
            if (str[j]=='(') level++;
            if (str[j]==')') level--;
        }
        std::string value(str + i + 1, j-i-1);
        const char *vstr = value.c_str();
        float real = 0;
        float imaginary = 0;
        int pos = -1;

        /* special case - annotation might be a polarization */
        if (_name == "P" && value.size() == 1 && *vstr >= 'A' && *vstr <= 'Z') {
            pos = 1;
            if (value == "H") {}
            else if (value == "V") { real = pi; }
            else if (value == "D") { real = pi / 2; }
            else if (value == "A") {
                real = pi / 2;
                imaginary = pi;
            }
            else if (value == "R") {
                real = pi / 2;
                imaginary = 3 * pi / 2;
            }
            else if (value == "L") {
                real = pi / 2;
                imaginary = pi / 2;
            }
            else {
                throw std::invalid_argument("invalid annotation (unknown polarization value)");
            }
        }
        /* ({DOUBLE},{DOUBLE}) */
        if (sscanf(vstr, "(%f,%f)%n", &real, &imaginary, &pos) == 2) {
        }
            /* {DOUBLE} or {COMPLEX} */
        else if (sscanf(vstr, "%f%n", &real, &pos) == 1) {
            vstr += pos;
            pos = 0;
            if (*vstr == 'i' || *vstr == 'j') {
                imaginary = real;
                real = 0;
                pos = 1;
            } else if (*vstr) {
                int sign = 0;
                if (*vstr == '+') {
                    sign = 1;
                    vstr += 1;
                }
                else if (*vstr == '-') {
                    sign = -1;
                    vstr += 1;
                }
                if (sign && sscanf(vstr, "%f%n", &imaginary, &pos) == 1) {
                    vstr += pos;
                    imaginary *= sign;
                    if (vstr[0] == 'i' || vstr[0] == 'j') {
                        pos = 1;
                    } else {
                        pos = -1;
                    }
                }
            }
        }
        if (pos == -1 || vstr[pos])
            throw std::invalid_argument("invalid annotation (cannot parse value)");
        if (this->find(_name) != this->end())
            throw std::invalid_argument("invalid annotation (duplicate tag)");
        (*this)[_name] = std::complex<float>(real, imaginary);
        str += j;
        if (*str == ',') str += 1;
    } while(*str);
}

std::string annotation::str_value(const std::string &tag) const {
    std::stringstream s;
    auto value = this->at(tag);
    bool special_annot = false;
    if (tag == "P") {
        special_annot = true;
        if (value == std::complex<float>(0)) { s << "H"; }
        else if (value == std::complex<float>(pi)) { s << "V"; }
        else if (value == std::complex<float>(pi / 2)) { s << "D"; }
        else if (value == std::complex<float>(pi / 2, pi)) { s << "A"; }
        else if (value == std::complex<float>(pi / 2, pi / 2)) { s << "L"; }
        else if (value == std::complex<float>(pi / 2, 3 * pi / 2)) { s << "R"; }
        else special_annot = false;
    }
    if (!special_annot) {
        if (value.imag() == 0) {
            s << value.real();
        } else {
            s << "(" << value.real() << "," << value.imag() << ")";
        }
    }
    return s.str();
}

std::string annotation::to_str() const {
    std::stringstream s;
    bool first = true;
    for(auto nv_iter: (*this)) {
        if (!first) s << ","; else first=false;
        s << nv_iter.first << ":";
        s << str_value(nv_iter.first);
    }
    return s.str();
}

bool annotation::compatible_annotation(const annotation &add_annot, annotation &new_annot) const {
    new_annot = *this;
    for (auto const &iter: add_annot) {
        if (iter.first == "P") continue;
        if (this->find(iter.first) == this->end())
            new_annot[iter.first] = iter.second;
        else {
            if (this->at(iter.first) != iter.second)
                return false;
        }
    }
    return true;
}

std::complex<float> annotation::get(const std::string &tag, std::complex<float> def) const {
    if (find(tag) == this->end()) return def;
    return at(tag);
}
