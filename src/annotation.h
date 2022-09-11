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

#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <map>
#include <string>
#include <complex>

class annotation : public std::map<std::string, std::complex<float>> {
    public:
        /* empty annotation */
        annotation() {}
        /* read a key-value KEY1:VALUE1,KEY2:VALUE2,... annotation */
        explicit annotation(const char *str);
        annotation(const std::string &name, const std::complex<float> &value) { (*this)[name] = value; }
        std::string to_str() const;
        bool has_tag(const std::string &name) const {
            return this->find(name) != this->end();
        }
        bool has_polarization() const {
            return has_tag("P");
        }
        bool operator==(const annotation &b) {
            if (b.size() != this->size()) return false;
            for(auto it: *this) {
                if (b.find(it.first) == b.end()) return false;
                if (it.second != b.at(it.first)) return false;
            }
            return true;
        }
        bool contains(const std::string &tag) const { return find(tag) != end(); }
        std::complex<float> get(const std::string &tag, std::complex<float> def) const;
        std::string str_value(const std::string &tag) const;
        bool compatible_annotation(const annotation &add_annot, annotation &new_annot) const;
};

#endif
