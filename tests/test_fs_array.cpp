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

#include <catch2/catch.hpp>
#include "../src/fs_array.h"
#include "../src/fs_map.h"

SCENARIO("Testing FS Array") {
    GIVEN("Building arrays") {
        WHEN("Instantiating with params") {
            auto m = GENERATE(2, 5);
            auto n = GENERATE(0, 1, 2);
            fs_array fsa(m, n);
            THEN("Array have the right size") {
                REQUIRE(fsa.get_m() == m);
                REQUIRE(fsa.get_n() == n);
            }
        }
        WHEN("specific values - checking count and internal structure") {
            fs_array fsa1(20, 10);
            REQUIRE(fsa1.count() == 20030010);

            fs_array fsa0(16, 0);
            REQUIRE(fsa0.size() == 0);
            REQUIRE(fsa0.count() == 1);
            std::vector<int> v(16);
            int niter = 0;
            for(fockstate fs:fsa0) {
                REQUIRE(fsa0[0] == fockstate(v));
                niter++;
            }
            REQUIRE(niter==1);
            REQUIRE(fsa0[0] == fockstate(v));
            REQUIRE(fsa0.find(fockstate(v)) == fsa0.begin());
            v[0] = 1;
            REQUIRE(fsa0.find(fockstate(v)) == fsa0.end());

            fs_array fsa3(5, 3);
            REQUIRE(fsa3.size() == 35 * 3);
            REQUIRE(fsa3.count() == 35);
            std::vector<int> v3(5);
            REQUIRE(fsa3.find(fockstate(v3)) == fsa3.end());
            v3[2] = 3;
            fs_array::const_iterator it = fsa3.find(fockstate(v3));
            REQUIRE(it.idx == 25);
            REQUIRE((*it).to_vect() == v3);
            v3[2] = 2;
            it = fsa3.find(fockstate(v3));
            REQUIRE(fsa3.find(fockstate(v3)) == fsa3.end());
        }
    }
    SECTION("iterating") {
        fs_array fsa(3,2);
        std::vector<std::string> expected{"|2,0,0>",
                                          "|1,1,0>",
                                          "|1,0,1>",
                                          "|0,2,0>",
                                          "|0,1,1>",
                                          "|0,0,2>"};
        size_t idx=0;
        for(auto fs: fsa) {
            REQUIRE(fs.to_str()==expected[idx++]);
        }
        REQUIRE(idx==expected.size());
        fsa=fs_array(2, 0);
        expected=std::vector<std::string>{"|0,0>"};
        idx=0;
        for(auto fs: fsa) {
            REQUIRE(fs.to_str()==expected[idx++]);
        }
        REQUIRE(idx==expected.size());
    }
    SECTION("finding in fsa") {
        fs_array fsa(2, 1);
        REQUIRE(fsa.find_idx(fockstate(std::vector<int>{1,0})) == 0);
        REQUIRE(fsa.find_idx(fockstate(std::vector<int>{0,1})) == 1);
    }
    SECTION("using a fs mask") {
        fs_mask mask(5, 3, " 1 1 0");
        fs_array fsa(5, 3, mask);
        REQUIRE(fsa.count() == 3);
        std::vector<int> v(5);
        fs_array::const_iterator it = fsa.find(fockstate(v));
        REQUIRE(it == fsa.end());
        v[2] = 3;
        /* it is in the space, but not in this constrained space */
        REQUIRE(it == fsa.end());
        v[1] = 1;
        v[2] = 1;
        v[3] = 1;
        it = fsa.find(fockstate(v));
        REQUIRE(it != fsa.end());
        REQUIRE((*it).to_vect() == v);
        v[0] = 1;
        v[3] = 0;
        it = fsa.find(fockstate(v));
        REQUIRE(it == fsa.end());
    }
    SECTION("builds a fsm from regular fsa") {
        WHEN("5 modes - storage on 1 byte") {
            fs_array fsa_parent(5, 2);
            fs_array fsa_child(5, 3);
            fs_map fsm(fsa_child, fsa_parent, false);
            REQUIRE(fsm.count() == fsa_parent.count());
            REQUIRE(fsm.count()*5 == fsm.size());
            fockstate fs = fockstate(std::vector<int>{0, 1, 1, 0, 0});
            unsigned long long idx = fsa_parent.find(fs).idx;
            REQUIRE(fsa_parent[idx].to_str() == "|0,1,1,0,0>");
            REQUIRE(fsa_child[fsm.get(idx, 0)].to_str() == "|1,1,1,0,0>");
            REQUIRE(fsa_child[fsm.get(idx, 1)].to_str() == "|0,2,1,0,0>");
            REQUIRE(fsa_child[fsm.get(idx, 2)].to_str() == "|0,1,2,0,0>");
            REQUIRE(fsa_child[fsm.get(idx, 3)].to_str() == "|0,1,1,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 4)].to_str() == "|0,1,1,0,1>");
        }
        WHEN("9 modes - storage on 2 bytes - no mask") {
            fs_array fsa_parent(9, 3);
            fs_array fsa_child(9, 4);
            fs_map fsm(fsa_child, fsa_parent, false);
            REQUIRE(fsm.count() == fsa_parent.count());
            REQUIRE(fsm.count()*9*2 == fsm.size());
            fockstate fs = fockstate(std::vector<int>{0, 1, 0, 0, 1, 0, 0, 1, 0});
            unsigned long long idx = fsa_parent.find(fs).idx;
            REQUIRE(fsa_parent[idx].to_str() == "|0,1,0,0,1,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 0)].to_str() == "|1,1,0,0,1,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 1)].to_str() == "|0,2,0,0,1,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 2)].to_str() == "|0,1,1,0,1,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 3)].to_str() == "|0,1,0,1,1,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 4)].to_str() == "|0,1,0,0,2,0,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 5)].to_str() == "|0,1,0,0,1,1,0,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 6)].to_str() == "|0,1,0,0,1,0,1,1,0>");
            REQUIRE(fsa_child[fsm.get(idx, 7)].to_str() == "|0,1,0,0,1,0,0,2,0>");
            REQUIRE(fsa_child[fsm.get(idx, 8)].to_str() == "|0,1,0,0,1,0,0,1,1>");
        }
        WHEN("9 modes - with a mask") {
            fs_mask mask(9, 4, "1       1");
            fs_array fsa_parent(9, 3, mask);
            fs_array fsa_child(9, 4, mask);
            fs_map fsm(fsa_child, fsa_parent, false);
            fockstate fs = fockstate(std::vector<int>{0, 1, 0, 0, 1, 0, 0, 1, 0});
            REQUIRE(fsa_parent.find(fs) == fsa_parent.end());
            fs = fockstate(std::vector<int>{1, 0, 0, 0, 1, 0, 0, 1, 0});
            REQUIRE(fsa_parent.find(fs) != fsa_parent.end());
            unsigned long long idx = fsa_parent.find(fs).idx;
            REQUIRE(fsa_parent[idx].to_str() == "|1,0,0,0,1,0,0,1,0>");
            REQUIRE(fsm.get(idx, 0) == fs_npos);
            REQUIRE(fsm.get(idx, 1) == fs_npos);
            REQUIRE(fsm.get(idx, 2) == fs_npos);
            REQUIRE(fsm.get(idx, 3) == fs_npos);
            REQUIRE(fsm.get(idx, 4) == fs_npos);
            REQUIRE(fsm.get(idx, 5) == fs_npos);
            REQUIRE(fsm.get(idx, 6) == fs_npos);
            REQUIRE(fsm.get(idx, 7) == fs_npos);
            REQUIRE(fsm.get(idx, 8) != fs_npos);
            REQUIRE(fsa_child[fsm.get(idx, 8)].to_str() == "|1,0,0,0,1,0,0,1,1>");
        }
    }
    SECTION("test coefficient normalization") {
        WHEN("with 3 photons") {
            fs_array fsa(3, 3);
            REQUIRE(fsa.count() == 10);
            std::vector<std::complex<double>> coefs(10);
            for(auto i=0; i<10; i++) coefs[i] = 1;
            fsa.norm_coefs(coefs.data());
            int i=0;
            REQUIRE(fsa[i].to_str() == "|3,0,0>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)6)));
            REQUIRE(fsa[i].to_str() == "|2,1,0>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|2,0,1>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|1,2,0>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|1,1,1>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)1)));
            REQUIRE(fsa[i].to_str() == "|1,0,2>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|0,3,0>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)6)));
            REQUIRE(fsa[i].to_str() == "|0,2,1>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|0,1,2>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)2)));
            REQUIRE(fsa[i].to_str() == "|0,0,3>");
            REQUIRE(coefs[i++].real() == Approx(1*sqrt((double)6)));
        }
    }
}
