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
        _name = "";
        return;
    }
    /* annotation format is [A-Za-z0-9_]+:(\({DOUBLE},{DOUBLE}\)|{DOUBLE}|{COMPLEX}\) or P:[HVADLR] */
    unsigned int i=0;
    for(;(str[i] >= 'A' && str[i] <= 'Z') ||
          (str[i] >= 'a' && str[i] <= 'z') ||
          (i > 0 && str[i] >= '0' && str[i] <= '9') ||
          (str[i] == '_'); i++) {}
    if (!str[i])
        throw std::invalid_argument("invalid annotation (no key-value separator)");
    if (str[i] != ':')
        throw std::invalid_argument("invalid annotation (invalid key format)");
    _name = std::string(str, 0, i);
    std::string value(str+i+1);
    str = value.c_str();
    float real = 0;
    float imaginary = 0;
    int pos = -1;

    /* special case - annotation might be a polarization */
    if (_name == "P" && value.size() == 1 && *str >= 'A' && *str <= 'Z') {
        pos = 1;
        if (value == "H") {}
        else if (value == "V") { real = pi; }
        else if (value == "D") { real = pi/2; }
        else if (value == "A") { real = pi/2; imaginary = pi; }
        else if (value == "R") { real = pi/2; imaginary = 3*pi/2; }
        else if (value == "L") { real = pi/2; imaginary = pi/2; }
        else {
            throw std::invalid_argument("invalid annotation (unknown polarization value)");
        }
    }
    /* ({DOUBLE},{DOUBLE}) */
    if (sscanf(str, "(%f,%f)%n", &real, &imaginary, &pos) == 2) {
    }
    /* {DOUBLE} or {COMPLEX} */
    else if (sscanf(str, "%f%n", &real, &pos) == 1) {
        str += pos;
        pos = 0;
        if (*str == 'i' || *str == 'j') {
            imaginary = real;
            real = 0;
            pos = 1;
        }
        else if (*str) {
            int sign=0;
            if (*str == '+') { sign = 1; str += 1; }
            else if (*str == '-') { sign = -1; str += 1; }
            if (sign && sscanf(str, "%f%n", &imaginary, &pos) == 1) {
                str += 1;
                imaginary *= sign;
            }
        }
    }
    if (pos == -1 || str[pos])
        throw std::invalid_argument("invalid annotation (cannot parse value)");
    _value.real(real);
    _value.imag(imaginary);
}

std::string annotation::to_str() const {
    std::stringstream s;
    if (!_name.empty()) {
        s << _name << ":";
        bool special_annot = false;
        if (_name == "P") {
            special_annot = true;
            if (_value == std::complex<float>(0)) { s << "H"; }
            else if (_value == std::complex<float>(pi)) { s << "V"; }
            else if (_value == std::complex<float>(pi / 2)) { s << "D"; }
            else if (_value == std::complex<float>(pi / 2, pi)) { s << "A"; }
            else if (_value == std::complex<float>(pi / 2, pi / 2)) { s << "L"; }
            else if (_value == std::complex<float>(pi / 2, 3 * pi / 2)) { s << "R"; }
            else special_annot = false;
        }
        if (!special_annot) {
            if (_value.imag() == 0) {
                s << _value.real();
            } else {
                s << "(" << _value.real() << "," << _value.imag() << ")";
            }
        }
    }
    return s.str();
}
